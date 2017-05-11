#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"
#include "SAMD21DAC.h"

#undef ENABLE

static DmaComponent* apps[DMA_DESCRIPTOR_COUNT]= {NULL};

extern "C" void DMAC_Handler( void )
{
    uint32_t oldChannel = DMAC->CHID.bit.ID;

    int channel = DMAC->INTPEND.bit.ID;
    DMAC->CHID.bit.ID = channel;
    DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL;

    if(apps[channel] != NULL)
        apps[channel]->dmaTransferComplete();

    DMAC->CHID.bit.ID = oldChannel;
}

/**
 * Base implementation of a DMA callback
 */
void DmaComponent::dmaTransferComplete()
{
}

SAMD21DMAC::SAMD21DMAC()
{
    uint32_t ptr = (uint32_t)descriptorsBuffer;
    while (ptr & (DMA_DESCRIPTOR_ALIGNMENT - 1))
        ptr++;
    descriptors = (DmacDescriptor*)ptr;

    memclr(descriptors, sizeof(DmacDescriptor) * (DMA_DESCRIPTOR_COUNT * 2));

    // Set up to DMA Controller
    this->disable();

    PM->APBBMASK.reg |= 0x10;                   // Enable the DMAC clock.
    PM->AHBMASK.reg |= 0x20;                    // Enable the DMAC clock.

    DMAC->CTRL.bit.LVLEN0 = 1;                  // Allow all DMA priorities.
    DMAC->CTRL.bit.LVLEN1 = 1;                  // Allow all DMA priorities.
    DMAC->CTRL.bit.LVLEN2 = 1;                  // Allow all DMA priorities.
    DMAC->CTRL.bit.LVLEN3 = 1;                  // Allow all DMA priorities.

    DMAC->CRCCTRL.reg = 0;                      // Disable all CRC expectations

    DMAC->BASEADDR.reg = (uint32_t) &descriptors[DMA_DESCRIPTOR_COUNT];      // Initialise Descriptor table                
    DMAC->WRBADDR.reg = (uint32_t) &descriptors[0];                          // initialise Writeback table 

    this->enable();

    NVIC_EnableIRQ(DMAC_IRQn);
    NVIC_SetPriority(DMAC_IRQn, 1);
}

void SAMD21DMAC::enable()
{
    DMAC->CTRL.bit.DMAENABLE = 1;               // Enable controller.
    DMAC->CTRL.bit.CRCENABLE = 1;               // Disable CRC checking.
}

void SAMD21DMAC::disable()
{
    DMAC->CTRL.bit.DMAENABLE = 0;               // Diable controller, just while we configure it.
    DMAC->CTRL.bit.CRCENABLE = 0;               // Disable CRC checking.
}

/**
 * Provides the SAMD21 specific DMA descriptor for the given channel number
 * @return a valid DMA decriptor, matching a previously allocated channel.
 */
DmacDescriptor& SAMD21DMAC::getDescriptor(int channel)
{
    if (channel < DMA_DESCRIPTOR_COUNT)
        return descriptors[channel+DMA_DESCRIPTOR_COUNT];

    return descriptors[0];
}

/**
 * Allocates an unused DMA channel, if one is available.
 * @return a valid channel descriptor in the range 1..DMA_DESCRIPTOR_COUNT, or DEVICE_NO_RESOURCES otherwise.
 */
int SAMD21DMAC::allocateChannel()
{
    for (int i=0; i<DMA_DESCRIPTOR_COUNT; i++)
    {
        if (!descriptors[i+DMA_DESCRIPTOR_COUNT].BTCTRL.bit.VALID)
        {
            descriptors[i+DMA_DESCRIPTOR_COUNT].BTCTRL.bit.VALID = 1;
            return i;
        }
    }

    return DEVICE_NO_RESOURCES;
}

/**
 * Registers a component to receive low level, hardware interrupt upon DMA transfer completion
 *
 * @param channel the DMA channel that the component is interested in.
 * @param component the component that wishes to receive the interrupt.
 *
 * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the channel number is invalid.
 */
int SAMD21DMAC::onTransferComplete(int channel, DmaComponent *component)
{
    if (channel >= DMA_DESCRIPTOR_COUNT)
        return DEVICE_INVALID_PARAMETER;

    apps[channel] = component;
    return DEVICE_OK;
}

#if CONFIG_ENABLED(DEVICE_DBG)

void SAMD21DMAC::showDescriptor(DmacDescriptor *desc)
{
    SERIAL_DEBUG->printf("DESC: %p\n", desc);
    SERIAL_DEBUG->printf("DESC->SRCADDR: %p\n", &desc->SRCADDR);
    SERIAL_DEBUG->printf("DESC->DSTADDR: %p\n", &desc->DSTADDR);

    SERIAL_DEBUG->printf("STESIZE: %d\n", desc->BTCTRL.bit.STEPSIZE);
    SERIAL_DEBUG->printf("DSTINC: %d\n", desc->BTCTRL.bit.DSTINC);
    SERIAL_DEBUG->printf("SRCINC: %d\n", desc->BTCTRL.bit.SRCINC);
    SERIAL_DEBUG->printf("BEATSIZE: %d\n", desc->BTCTRL.bit.BEATSIZE);
    SERIAL_DEBUG->printf("BLOCKACT: %d\n", desc->BTCTRL.bit.BLOCKACT);
    SERIAL_DEBUG->printf("EVOSEL: %d\n", desc->BTCTRL.bit.EVOSEL);
    SERIAL_DEBUG->printf("VALID: %d\n", desc->BTCTRL.bit.VALID);

    SERIAL_DEBUG->printf("BTCNT: %d\n", desc->BTCNT.bit.BTCNT);
    SERIAL_DEBUG->printf("SRCADDR: %p\n", desc->SRCADDR.bit.SRCADDR);
    SERIAL_DEBUG->printf("DSTADDR: %p\n", desc->DSTADDR.bit.DSTADDR);
    SERIAL_DEBUG->printf("DESCADDR: %p\n", desc->DESCADDR.bit.DESCADDR);
}

void SAMD21DMAC::showRegisters()
{
    SERIAL_DEBUG->printf("BASEADDR: 0x%.8x[%p]\n", DMAC->BASEADDR.reg, &descriptors[1]);
    SERIAL_DEBUG->printf("WRBADDR: 0x%.8x [%p]\n", DMAC->WRBADDR.reg, &descriptors[0]);
    SERIAL_DEBUG->printf("CTRL: 0x%.2x\n", DMAC->CTRL.reg);
    SERIAL_DEBUG->printf("PENDCH: 0x%.8x\n", DMAC->PENDCH.reg);
    DMAC->CHID.bit.ID = 0;                      // Select channel 0 
    SERIAL_DEBUG->printf("CHCTRLA: 0x%.8x\n", DMAC->CHCTRLA.reg);
    SERIAL_DEBUG->printf("CHCTRLB: 0x%.8x\n", DMAC->CHCTRLB.reg);
    SERIAL_DEBUG->printf("INTPEND: 0x%.8x\n", DMAC->INTPEND.reg);
    SERIAL_DEBUG->printf("ACTIVE: 0x%.8x\n", DMAC->ACTIVE.reg);
    SERIAL_DEBUG->printf("BUSYCH: 0x%.8x\n", DMAC->BUSYCH.reg);

    SERIAL_DEBUG->printf("APBBMASK: 0x%.8x\n", PM->APBBMASK.reg);
    SERIAL_DEBUG->printf("AHBMASK: 0x%.2x\n", PM->AHBMASK.reg);
}

#endif

