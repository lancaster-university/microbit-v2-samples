#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"
#include "SAMD21DAC.h"

#undef ENABLE

SAMD21DAC::SAMD21DAC(DevicePin &pin, SAMD21DMAC &dma, uint16_t id) : dmac(dma)
{
    this->id = id;

    // Put the pin into output mode.
    pin.setDigitalValue(0);

    // move the pin to DAC mode.
    uint32_t v = 0x51410000 | 0x01 << pin.name;
    PORT->Group[0].WRCONFIG.reg = v;

    // Enbale the DAC bus clock (CLK_DAC_APB | CLK_EVSYS_APB | CLK_TC3_APB) 
    PM->APBCMASK.reg |= 0x00040802;

    // Bring up the necessry system clocks...

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

    // Configure TC5 for a 1uS tick 10KHz overflow
    TC3->COUNT16.CTRLA.reg = 0x0B20;        // 16 bit 8:1 prescaler
    TC3->COUNT16.CTRLC.reg = 0x00;          // compare mode
    TC3->COUNT16.CC[0].reg = 100;           // 100 uS period
    TC3->COUNT16.EVCTRL.reg = 0x1100;       // Enable periodoverflow events.
    TC3->COUNT16.CTRLBCLR.bit.DIR = 1;      // Start the timer
    TC3->COUNT16.CTRLA.bit.ENABLE = 1;      // Start the timer

    // Enable the DAC.
    DAC->CTRLA.reg = 0x00;
    DAC->EVCTRL.reg = 0x03;
    DAC->CTRLB.reg = 0x5B;
    DAC->CTRLA.reg = 0x06;

    // Initialise a DMA channel
    dmac.disable();

    dmaChannel = dmac.allocateChannel();
    SERIAL_DEBUG->printf("DAC: ALLOCATED DMA CHANNEL: %d\n", dmaChannel);

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

        SERIAL_DEBUG->printf("DAC: ALLOCATED DMA CHANNEL: %d\n", dmaChannel);
    }

    dmac.enable();
}

int SAMD21DAC::play(const uint16_t *buffer, int length)
{
    if (dmaChannel == DEVICE_NO_RESOURCES)
        return DEVICE_NO_RESOURCES;

    DmacDescriptor &descriptor = dmac.getDescriptor(dmaChannel);

    descriptor.SRCADDR.reg = ((uint32_t) buffer) + ((length) * 2);
    descriptor.BTCNT.bit.BTCNT = length;

    // Enable our DMA channel.
    DMAC->CHID.bit.ID = dmaChannel;             
    DMAC->CHCTRLA.bit.ENABLE = 1;                

    return DEVICE_OK;
}

void SAMD21DAC::setValue(int value)
{
    DAC->DATA.reg = value;
}

int SAMD21DAC::getValue()
{
    return DAC->DATA.reg;
}

extern void debug_flip();

/**
 * Base implementation of a DMA callback
 */
void SAMD21DAC::dmaTransferComplete()
{
    // restart the last transfer
    DMAC->CHID.bit.ID = dmaChannel;             
    DMAC->CHCTRLA.bit.ENABLE = 1;                
}

