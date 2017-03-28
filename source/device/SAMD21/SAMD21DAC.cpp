#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"
#include "SAMD21DAC.h"

#undef ENABLE

void show_regs()
{
    /* Enable all EIC hardware modules. */
    SERIAL_DEBUG->printf("CTRLA: 0x%.2x\n  CTRLB: 0x%.2x\n  EVCTRL: 0x%.2x\n", DAC->CTRLA.reg, DAC->CTRLB.reg, DAC->EVCTRL.reg);

    SERIAL_DEBUG->printf("PM: 0x%.8x\n", PM->APBAMASK.reg);


    /* Enable the generic clock */
    //GCLK->CLKCTRL.reg |= GCLK_CLKCTRL_CLKEN;
    
    *((uint8_t*)&GCLK->CLKCTRL.reg) = 0x21;
    SERIAL_DEBUG->printf("DAC_CLKCTRL: 0x%.8x\n", GCLK->CLKCTRL.reg);
    *((uint8_t*)&GCLK->GENCTRL.reg) = 0x08;
    SERIAL_DEBUG->printf("DAC_GENCTRL: 0x%.8x\n", GCLK->GENCTRL.reg);
    *((uint8_t*)&GCLK->GENDIV.reg) = 0x08;
    SERIAL_DEBUG->printf("DAC_GENDIV: 0x%.8x\n", GCLK->GENDIV.reg);

    SERIAL_DEBUG->printf("APBCMASK: 0x%.8x\n", PM->APBCMASK.reg);
    SERIAL_DEBUG->printf("APBCSEL: 0x%.2x\n", PM->APBCSEL.reg);
 
    //SERIAL_DEBUG->.printf("PINMUXEN: %.2X\n", PORT->Group[0].PINCFG[4].bit.PMUXEN);
    //SERIAL_DEBUG->.printf("PINMUX: %.2X\n", PORT->Group[0].PMUX[2].bit.PMUXE);
}

SAMD21DAC::SAMD21DAC(DevicePin &pin, SAMD21DMAC &dma, uint16_t id) : dmac(dma)
{
    this->id = id;

    // Put the pin into output mode.
    pin.setDigitalValue(0);

    // move the pin to DAC mode.
    uint32_t v = 0x51410000 | 0x01 << pin.name;
    PORT->Group[0].WRCONFIG.reg = v;

    // Enbale the DAC bus clock (CLK_DAC_APB) 
    PM->APBCMASK.reg |= 0x00040000;

    // Bring up the necessry system clocks...
    // Select the DAC clock source 
    GCLK->CLKCTRL.bit.ID = 0x21; 

    // Configure a new generator at 44KHz.
    GCLK->GENCTRL.bit.ID = 0x08;      

    //GCLK->GENCTRL.bit.SRC = 0x06;   // 8MHz source
    GCLK->GENCTRL.bit.SRC = 0x04;   // 32KHz source
    GCLK->GENCTRL.bit.DIVSEL = 0;   // linear clock divide 
    GCLK->GENCTRL.bit.OE = 0;       // Do not output to GPIO 
    GCLK->GENCTRL.bit.OOV = 0;      // Do not output to GPIO 
    GCLK->GENCTRL.bit.IDC = 1;      // improve accuracy 
    GCLK->GENCTRL.bit.RUNSTDBY = 1;    // improve accuracy 
    GCLK->GENCTRL.bit.GENEN = 1;    // improve accuracy 

    GCLK->GENDIV.bit.ID = 0x08;      
    //GCLK->GENDIV.bit.DIV = 181;     // Configure for 44.1KHz
    GCLK->GENDIV.bit.DIV = 0;     // Configure for 44.1KHz

    GCLK->CLKCTRL.bit.GEN = 0x08;      
    GCLK->CLKCTRL.bit.CLKEN = 1;    // Enable clock

    DAC->CTRLB.reg = 0x53;
    DAC->CTRLA.reg = 0x06;
    DAC->EVCTRL.reg = 0x03;

    GCLK->CLKCTRL.bit.ID = 0x21; 
    GCLK->CLKCTRL.bit.CLKEN = 1;    // Enable clock

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
        descriptor.BTCTRL.bit.EVOSEL = 0;       // Strobe events after every BEAT transfer
        descriptor.BTCTRL.bit.VALID = 1;        // Enable the descritor

        descriptor.BTCNT.bit.BTCNT = 0;
        descriptor.SRCADDR.reg = 0;
        descriptor.DSTADDR.reg = (uint32_t) &DAC->DATA.reg;
        descriptor.DESCADDR.reg = 0;

        DMAC->CHID.bit.ID = dmaChannel;             // Select our allocated channel

        DMAC->CHCTRLB.bit.CMD = 0;                  // No Command (yet)
        DMAC->CHCTRLB.bit.TRIGACT = 2;              // One trigger per beat transfer
        DMAC->CHCTRLB.bit.TRIGSRC = 0;              // Only softwate trigger (should be 0x28)
        DMAC->CHCTRLB.bit.LVL = 0;                  // Low priority transfer 
        DMAC->CHCTRLB.bit.EVOE = 0;                 // no events 
        DMAC->CHCTRLB.bit.EVIE = 0;                 // no events 
        DMAC->CHCTRLB.bit.EVACT = 0;                // no events 

        DMAC->CHINTENSET.bit.TCMPL = 1;             // Enable interrupt on completion.

        dmac.onTransferComplete(dmaChannel, this);

        SERIAL_DEBUG->printf("DAC: ALLOCATED DMA CHANNEL: %d\n", dmaChannel);
    }

    dmac.enable();
}

void showBuf(uint16_t *buf)
{
    for (int i=0; i<16; i++)
        SERIAL_DEBUG->printf("0x%.4X ", buf[i]);
    SERIAL_DEBUG->printf("\n");
}


void SAMD21DAC::play(uint16_t *buffer, int length)
{
    if (dmaChannel == DEVICE_NO_RESOURCES)
    {
        SERIAL_DEBUG->printf("PLAY: NO DMA CHANNEL\n");
        return;
    }

    DmacDescriptor &descriptor = dmac.getDescriptor(dmaChannel);

    descriptor.SRCADDR.reg = ((uint32_t) buffer) + (length * 2);
    descriptor.BTCNT.bit.BTCNT = length;

    // Enable our DMA channel.
    DMAC->CHID.bit.ID = dmaChannel;             
    DMAC->CHCTRLA.bit.ENABLE = 1;                

    // Start the transfer. For now, clock from the CPU (just for testing).
    for (int i=0; i<length; i++)
    {
        wait_us(100);
        DMAC->SWTRIGCTRL.bit.SWTRIG0 = 1;           // Start the DMA transfer!
    }
}

void SAMD21DAC::setValue(int value)
{
    DAC->DATA.reg = value;
}

int SAMD21DAC::getValue()
{
    return DAC->DATA.reg;
}

/**
 * Base implementation of a DMA callback
 */
void SAMD21DAC::dmaTransferComplete()
{
    SERIAL_DEBUG->printf("*");
}

