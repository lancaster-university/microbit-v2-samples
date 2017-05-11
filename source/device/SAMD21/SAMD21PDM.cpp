/*
The MIT License (MIT)

Copyright (c) 2016 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "SAMD21PDM.h"
#include "DevicePin.h"

#undef ENABLE

/**
 * An 8 bit PDM lookup table, used to reduce processing time.
 */
const int8_t pdmDecode[256] = {
#   define S(n) (2*(n)-8)
#   define B2(n) S(n),  S(n+1),  S(n+1),  S(n+2)
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
B6(0), B6(1), B6(1), B6(2)
};

/**
 * Update our reference to a downstream component.
 * Pass through any connect requests to our output buffer component.
 *
 * @param component The new downstream component for this PDM audio source.
 */
void SAMD21PDM::connect(DataSink& component)
{
    output.connect(component);
}

/**
 * Constructor for an instance of a PDM input (typically microphone),
 *
 * @param sd The pin the PDM data input is connected to.
 * @param sck The pin the PDM clock is conected to.
 * @param dma The DMA controller to use for data transfer.
 * @param sampleRate the rate at which samples are generated in the output buffer (in Hz)
 * @param clockRate the rate at which raw PDM data is extracted from the input source (in Hz)
 * @param id The id to use for the message bus when transmitting events.
 */
SAMD21PDM::SAMD21PDM(DevicePin &sd, DevicePin &sck, SAMD21DMAC &dma, int sampleRate, int clockRate, uint16_t id) : dmac(dma), output(*this)
{
    this->id = id;
    this->clockRate = clockRate;
    this->sampleRate = sampleRate;
    this->enabled = false;
    this->pdmCount = 0;
    this->outputBufferSize = 512;

    this->pdmDataBuffer = NULL;
    this->pdmReceiveBuffer = rawPDM1;

    buffer = ManagedBuffer(outputBufferSize);
    out = (int16_t *) &buffer[0];

    output.setBlocking(false);

    // Configure sd and sck pins as inputs
    sd.getDigitalValue();
    sck.setDigitalValue(0);

    // Move the pins into I2S PDM mode.
    PORT->Group[0].WRCONFIG.reg = (uint32_t ) (0x56030000 | (1 << sck.name));
    PORT->Group[0].WRCONFIG.reg = (uint32_t ) (0x56030000 | (1 << sd.name));

    // Enbale the I2S bus clock (CLK_I2S_APB)
    PM->APBCMASK.reg |= 0x00100000;
    
    // Configure the GCLK_I2S_0 clock source 
    GCLK->CLKCTRL.bit.ID = 0x23;             
    GCLK->CLKCTRL.bit.CLKEN = 0;    
    while(GCLK->CLKCTRL.bit.CLKEN);    

    // We run off the 8MHz clock, and clock divide in the I2S peripheral (to avoid using another clock generator)
    GCLK->CLKCTRL.bit.GEN = 0x01;   // 8MHz clock source
    GCLK->CLKCTRL.bit.CLKEN = 1;    // Enable clock

    // Configure a DMA channel
    dmac.disable();

    dmaChannel = dmac.allocateChannel();

    if (dmaChannel != DEVICE_NO_RESOURCES)
    {
        DmacDescriptor &descriptor = dmac.getDescriptor(dmaChannel);

        descriptor.BTCTRL.bit.STEPSIZE = 0;     // Unused
        descriptor.BTCTRL.bit.STEPSEL = 0;      // DMA step size if defined by the size of a BEAT transfer
        descriptor.BTCTRL.bit.DSTINC = 1;       // increment does apply to destintion address
        descriptor.BTCTRL.bit.SRCINC = 0;       // increment does not apply to source address 
        descriptor.BTCTRL.bit.BEATSIZE = 2;     // 32 bit wide transfer. 
        descriptor.BTCTRL.bit.BLOCKACT = 0;     // No action when transfer complete. 
        descriptor.BTCTRL.bit.EVOSEL = 3;       // Strobe events after every BEAT transfer
        descriptor.BTCTRL.bit.VALID = 1;        // Enable the descritor

        descriptor.BTCNT.bit.BTCNT = 0;
        descriptor.SRCADDR.reg = (uint32_t) &I2S->DATA[1].reg;
        descriptor.DSTADDR.reg = 0;
        descriptor.DESCADDR.reg = 0;

        DMAC->CHID.bit.ID = dmaChannel;             // Select our allocated channel

        DMAC->CHCTRLB.bit.CMD = 0;                  // No Command (yet)
        DMAC->CHCTRLB.bit.TRIGACT = 2;              // One trigger per beat transfer
        DMAC->CHCTRLB.bit.TRIGSRC = 0x2A;           // I2S RX1 trigger 
        DMAC->CHCTRLB.bit.LVL = 0;                  // Low priority transfer 
        DMAC->CHCTRLB.bit.EVOE = 0;                 // Enable output event on every BEAT 
        DMAC->CHCTRLB.bit.EVIE = 1;                 // Enable input event 
        DMAC->CHCTRLB.bit.EVACT = 0;                // Trigger DMA transfer on BEAT

        DMAC->CHINTENSET.bit.TCMPL = 1;             // Enable interrupt on completion.

        dmac.onTransferComplete(dmaChannel, this);
    }

    dmac.enable();

    // Configure for DMA enabled, single channel PDM input.
    int clockDivisor = 1;
    int cs = 8000000;
    while(cs >= clockRate && clockDivisor < 0x1f)
    {
        clockDivisor++;
        cs = 8000000 / clockDivisor;
    }

    // We want to run at least as fast as the requested speed, so scale up if needed.
    if (cs <= clockRate)
    {
        clockDivisor--;
        cs = 8000000 / clockDivisor;
    }

    // Record our actual clockRate, as it's useful for calculating sample window sizes etc.
    clockRate = cs;

    // pre-calculate the rates at which RAW samples will be converted int PCM (to speed up IRQ processing).
    this->overSamplingRate = clockRate / sampleRate;
    this->pdmCount = overSamplingRate >> 3;
    this->overSamplingErrorPeriod = SAMD21_PDM_BUFFER_SIZE / ((clockRate % sampleRate) / SAMD21_PDM_BUFFER_SIZE);         
    this->sampleRate = clockRate / overSamplingRate;

    // Disable I2S module while we configure it...
    I2S->CTRLA.reg = 0x00;      

    // Configure for a 32 bit wide receive, with a SCK clock generated from GCLK_I2S_0.
    I2S->CLKCTRL[0].reg = 0x00000007 | ((clockDivisor-1) << 19);         

    // Configure serializer for a 32 bit data word transferred in a single DMA operation, clocked by clock unit 0.
    I2S->SERCTRL[1].reg = 0x00000000;         

    // Enable I2S module.
    I2S->CTRLA.reg = 0x3E;

    // Create a listener to receive data ready events from our ISR.
    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->listen(id, SAMD21_PDM_DATA_READY, this, &SAMD21PDM::decimate);
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer SAMD21PDM::pull()
{
	return buffer;
}

/**
 * A garden variety CIC filter.
 * Based on a very well optimised and documented implementation from here:
 * https://curiouser.cheshireeng.com/2015/01/16/pdm-in-a-tiny-cpu/
 */
void SAMD21PDM::decimate(DeviceEvent)
{
    uint8_t *b = pdmDataBuffer;

    // Ensure we have a sane buffer
    if (pdmDataBuffer == NULL)
        return;

    for (int i=0; i < SAMD21_PDM_BUFFER_SIZE; i++)
    {
        // Now feed the 4 bit result to a second CIC with N=2, R=16, M=2
        // which has bit growth of 10, for a total of 14 significant bits
        // out. The counter scount is used to implement the decimation.
        s2_sum1 += pdmDecode[*b++];
        s2_sum2 += s2_sum1;

        if (--pdmCount == 0) 
        {
            int tmp1;
            int v = s2_sum2;

            tmp1 = v - s2_comb1_2;
            s2_comb1_2 = s2_comb1_1; 
            s2_comb1_1 = v;
            v = tmp1 - s2_comb2_2;
            s2_comb2_2 = s2_comb2_1; 
            s2_comb2_1 = tmp1;

            pdmCount = overSamplingRate >> 3;

            // Record a normalised 13 bit PCM sample.
            sum += v;
            samples++;
            *out = (v - avg);
            out++;
            
            // If our output buffer is full, schedule it to flow downstream.
            if (out == (int16_t *) (&buffer[0] + outputBufferSize))
            {
                if (invalid)
                {
                    invalid--;
                }
                else
                {
                    output.pullRequest(); 
                    buffer = ManagedBuffer(outputBufferSize);
                }

                out = (int16_t *) &buffer[0];

                avg = sum / samples;
                sum = 0;
                samples = 0;
            }
        }
    }

    // Record that we've completed processing.
    pdmDataBuffer = NULL;
}

void SAMD21PDM::dmaTransferComplete()
{
    // If the last puffer has already been processed, start processing this buffer.
    // otherwise, we're running behind for some reason, so drop this buffer.
    if (pdmDataBuffer == NULL)
    {
        pdmDataBuffer = pdmReceiveBuffer;
        DeviceEvent(id, SAMD21_PDM_DATA_READY);

        pdmReceiveBuffer = pdmReceiveBuffer == rawPDM1 ? rawPDM2 : rawPDM1;
    }

    // start the next DMA transfer, unless we've been asked to stop.
    if (enabled)
        startDMA();
}

/**
 * Enable this component
 */
void SAMD21PDM::enable()
{
    // If we're already running, nothing to do.
    if (enabled)
        return;

    // Initiate a DMA transfer.
    enabled = true;
    invalid = SAMD21_START_UP_DELAY;
    startDMA();
}

/**
 * Disable this component
 */
void SAMD21PDM::disable()
{
    // Schedule all DMA transfers to stop after the next DMA transaction completes.
    enabled = false;
}

/**
 * Initiate a DMA transfer into the raw data buffer.
 */
void SAMD21PDM::startDMA()
{
    // TODO: Determine if we can move these three lines into the constructor.
    DmacDescriptor &descriptor = dmac.getDescriptor(dmaChannel);
    descriptor.DSTADDR.reg = ((uint32_t) pdmReceiveBuffer) + SAMD21_PDM_BUFFER_SIZE;
    descriptor.BTCNT.bit.BTCNT = SAMD21_PDM_BUFFER_SIZE / 4;

    // Enable the DMA channel.
    DMAC->CHID.bit.ID = dmaChannel;             
    DMAC->CHCTRLA.bit.ENABLE = 1;                

    // Access the Data buffer once, to ensure we don't miss a DMA trigger...
    I2S->DATA[1].reg = I2S->DATA[1].reg;
}

