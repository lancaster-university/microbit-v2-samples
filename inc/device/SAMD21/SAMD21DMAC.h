#include "DeviceConfig.h"
#include "SystemClock.h"
#include "DevicePin.h"

#ifndef SAMD21DMAC_H
#define SAMD21DMAC_H

#define DMA_DESCRIPTOR_ALIGNMENT 16 // SAMD21 Datasheet 20.8.15 and 20.8.16
#define DMA_DESCRIPTOR_COUNT 4

class DmaComponent
{
    public:
    virtual void dmaTransferComplete();
};


class SAMD21DMAC
{
    // descriptors have to be 128 bit aligned - we allocate 16 more bytes, and set descriptors
    // at the right offset in descriptorsBuffer
    uint8_t descriptorsBuffer[sizeof(DmacDescriptor) * (DMA_DESCRIPTOR_COUNT + 1) + DMA_DESCRIPTOR_ALIGNMENT];
    DmacDescriptor *descriptors;

public:
    
    /**
      * Constructor for an instance of a DAC,
      */
    SAMD21DMAC();

    /**
     * Provides the SAMD21 specific DMA descriptor for the given channel number
     * @return a valid DMA decriptor, matching a previously allocated channel.
     */
    DmacDescriptor& getDescriptor(int channel);

    /**
     * Allocates an unused DMA channel, if one is available.
     * @return a valid channel descriptor in the range 1..DMA_DESCRIPTOR_COUNT, or DEVICE_NO_RESOURCES otherwise.
     */
    int allocateChannel();

    /**
     * Release a previously allocated channel.
     * @param channel the id of the channel to free.
     * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the channel is invalid.
     */
    int freeChannel(int channel);

    /**
     * Disables all confgures DMA activity. 
     * Typically required before configuring DMA descriptors and DMA channels.
     */
    void disable();

    /**
     * Enables all confgures DMA activity
     */
    void enable();

    /**
     * Registers a component to receive low level, hardware interrupt upon DMA transfer completion
     *
     * @param channel the DMA channel that the component is interested in.
     * @param component the component that wishes to receive the interrupt.
     *
     * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the channel number is invalid.
     */
    int onTransferComplete(int channel, DmaComponent *component);

#if CONFIG_ENABLED(DEVICE_DBG)
    void showDescriptor(DmacDescriptor *desc);
    void showRegisters();
#endif

};

#endif
