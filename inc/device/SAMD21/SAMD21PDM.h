#include "DeviceConfig.h"
#include "SystemClock.h"
#include "DevicePin.h"
#include "SAMD21DMAC.h"
#include "DataStream.h"

#ifndef SAMD21PDM_H
#define SAMD21PDM_H

class SAMD21PDM : public DeviceComponent, public DmaComponent, public DataSource
{

private:
	ManagedBuffer   buffer;
	uint32_t        sampleRate;
	uint32_t        bufferSize;
    int             dmaChannel;
    SAMD21DMAC      &dmac;

public:
   
	DataStream output;

    /**
      * Constructor for an instance of a PDM input (typically microphone),
      *
      * @param sd The pin the PDM data input is connected to.
      * @param sck The pin the PDM clock is conected to.
      * @param dma The DMA controller to use for data transfer.
      * @param sampleRate the rate at which this PDM source generates samples.
      * @param id The id to use for the message bus when transmitting events.
      */
    SAMD21PDM(DevicePin &sd, DevicePin &sck, SAMD21DMAC &dma, int sampleRate, uint16_t id = DEVICE_ID_SYSTEM_MICROPHONE);

	/**
	 * Provide the next available ManagedBuffer to our downstream caller, if available.
	 */
	virtual ManagedBuffer pull();

	/**
     * Update our reference to a downstream component.
	 */
	virtual void connect(DataSink &sink);

    /**
     * Interrupt callback when playback of DMA buffer has completed
     */
    virtual void dmaTransferComplete();
};

#endif
