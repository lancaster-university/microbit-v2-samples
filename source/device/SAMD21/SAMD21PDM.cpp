#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"
#include "SAMD21PDM.h"

#undef ENABLE

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
 * @param sampleRate the rate at which this PDM source generates samples.
 * @param id The id to use for the message bus when transmitting events.
 */
SAMD21PDM::SAMD21PDM(DevicePin &sd, DevicePin &sck, SAMD21DMAC &dma, int sampleRate, uint16_t id) : dmac(dma), output(*this)
{
    this->id = id;
    this->sampleRate = sampleRate;
	this->bufferSize = 512;

    // Configure sd and sck pins as inputs
    sd.getDigitalValue();
    sck.getDigitalValue();

    // TODO: move the pins into I2S PDM mode.
    //uint32_t v = 0x51410000 | 0x01 << pin.name;
    //PORT->Group[0].WRCONFIG.reg = v;

    // TODO: Configure Clock sources

    // Enbale the DAC bus clock (CLK_DAC_APB | CLK_EVSYS_APB | CLK_TC3_APB) 
    //PM->APBCMASK.reg |= 0x00040802;
    
#ifdef TODO
    // First the DAC clock source 
    GCLK->CLKCTRL.bit.ID = 0x21; 
    GCLK->CLKCTRL.bit.CLKEN = 0;    
    while(GCLK->CLKCTRL.bit.CLKEN);    

    // Configure a new clock generator to run at the DAC conversion frequency
    GCLK->GENCTRL.bit.ID = 0x08;      

    GCLK->GENCTRL.bit.SRC = 0x06; // OSC8M source
    GCLK->GENCTRL.bit.DIVSEL = 0;   // linear clock divide 
    GCLK->GENCTRL.bit.OE = 0;       // Do not output to GPIO 
    GCLK->GENCTRL.bit.OOV = 0;      // Do not output to GPIO 
    GCLK->GENCTRL.bit.IDC = 1;      // improve accuracy 
    GCLK->GENCTRL.bit.RUNSTDBY = 1;    // improve accuracy 
    GCLK->GENCTRL.bit.GENEN = 1;    // enable clock generator

    GCLK->GENDIV.bit.ID = 0x08;      
    GCLK->GENDIV.bit.DIV = 32;   

    GCLK->CLKCTRL.bit.GEN = 0x08;      
    GCLK->CLKCTRL.bit.CLKEN = 1;    // Enable clock

    // Next bring up the TC3 clock at 8MHz.
    GCLK->CLKCTRL.bit.ID = 0x1B;    // TC3 Clock
    GCLK->CLKCTRL.bit.CLKEN = 0;    
    while(GCLK->CLKCTRL.bit.CLKEN);    

    GCLK->CLKCTRL.bit.GEN = 0x03;   // 8MHz peripheral clock source
    GCLK->CLKCTRL.bit.CLKEN = 1;    // Enable clock
#endif

    // TODO: Configure DMA channel
#ifdef TODO
    dmac.disable();

    dmaChannel = dmac.allocateChannel();
#if CONFIG_ENABLED(CODAL_DMA_DBG)
    SERIAL_DEBUG->printf("DAC: ALLOCATED DMA CHANNEL: %d\n", dmaChannel);
#endif

    if (dmaChannel != DEVICE_NO_RESOURCES)
    {
        DmacDescriptor &descriptor = dmac.getDescriptor(dmaChannel);

        descriptor.BTCTRL.bit.STEPSIZE = 0;     // Auto increment address by 1 after each beat
        descriptor.BTCTRL.bit.STEPSEL = 0;      // increment applies to SOURCE address
        descriptor.BTCTRL.bit.DSTINC = 0;       // increment does not apply to destintion address
        descriptor.BTCTRL.bit.SRCINC = 1;       // increment does apply to source address 
        descriptor.BTCTRL.bit.BEATSIZE = 1;     // 16 bit wide transfer. 
        descriptor.BTCTRL.bit.BLOCKACT = 0;     // No action when transfer complete. 
        descriptor.BTCTRL.bit.EVOSEL = 3;       // Strobe events after every BEAT transfer
        descriptor.BTCTRL.bit.VALID = 1;        // Enable the descritor

        descriptor.BTCNT.bit.BTCNT = 0;
        descriptor.SRCADDR.reg = 0;
        descriptor.DSTADDR.reg = (uint32_t) &DAC->DATA.reg;
        descriptor.DESCADDR.reg = 0;

        DMAC->CHID.bit.ID = dmaChannel;             // Select our allocated channel

        DMAC->CHCTRLB.bit.CMD = 0;                  // No Command (yet)
        DMAC->CHCTRLB.bit.TRIGACT = 2;              // One trigger per beat transfer
        DMAC->CHCTRLB.bit.TRIGSRC = 0x18;           // TC3 overflow trigger (could also be 0x22? match Compare C0?)
        DMAC->CHCTRLB.bit.LVL = 0;                  // Low priority transfer 
        DMAC->CHCTRLB.bit.EVOE = 0;                 // Enable output event on every BEAT 
        DMAC->CHCTRLB.bit.EVIE = 1;                 // Enable input event 
        DMAC->CHCTRLB.bit.EVACT = 0;                // Trigger DMA transfer on BEAT

        DMAC->CHINTENSET.bit.TCMPL = 1;             // Enable interrupt on completion.

        dmac.onTransferComplete(dmaChannel, this);
    }

    dmac.enable();
#endif

    //TODO: Configure for DMA enabled, single channel PDM input.

}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer SAMD21PDM::pull()
{
	return buffer;
}

/**
 * Base implementation of a DMA callback
 */
void SAMD21PDM::dmaTransferComplete()
{
    // Attempt to push data downstream.
    output.pullRequest();

    // Configure next buffer for receiving.
    buffer = ManagedBuffer(bufferSize);

    // TODO: re-enable DMA transfer.
    DmacDescriptor &descriptor = dmac.getDescriptor(dmaChannel);

    descriptor.SRCADDR.reg = ((uint32_t) &buffer[0]) + (buffer.length());
    descriptor.BTCNT.bit.BTCNT = buffer.length()/2;

    // Enable the DMA channel.
    DMAC->CHID.bit.ID = dmaChannel;             
    DMAC->CHCTRLA.bit.ENABLE = 1;                
}

