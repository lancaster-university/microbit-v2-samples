#include "DeviceConfig.h"
#include "SystemClock.h"
#include "DevicePin.h"

#ifndef SAMD21DMAC_H
#define SAMD21DMAC_H

#define DMA_DESCRIPTOR_COUNT 2

class SAMD21DMAC
{
    DmacDescriptor descriptors[DMA_DESCRIPTOR_COUNT+1];

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

    // DEBUG HELPERS
    void showDescriptor(DmacDescriptor *desc);
    void showRegisters();

};

#endif
