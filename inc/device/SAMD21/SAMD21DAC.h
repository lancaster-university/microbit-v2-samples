#include "DeviceConfig.h"
#include "SystemClock.h"
#include "DevicePin.h"
#include "SAMD21DMAC.h"
#include "DataStream.h"

#ifndef SAMD21DAC_H
#define SAMD21DAC_H

class SAMD21DAC : public DeviceComponent, public DmaComponent, public DataSink
{

private:
    SAMD21DMAC  &dmac;
    int         dmaChannel;
    bool        active;
    int         dataReady;

public:
   
    // The stream component that is serving our data 
    DataSource  &upstream;
    ManagedBuffer output;

    /**
      * Constructor for an instance of a DAC,
      *
      * @param pin The pin this DAC shoudl output to.
      * @param id The id to use for the message bus when transmitting events.
      */
    SAMD21DAC(DevicePin &pin, SAMD21DMAC &dma, DataSource &source, uint16_t id = DEVICE_ID_SYSTEM_DAC);

    /**
     * Callback provided when data is ready.
     */
	virtual int pullRequest();

    /**
     * Pull down a buffer from upstream, and schedule a DMA transfer from it.
     */
    int pull();

    void setValue(int value);
    int getValue();
    int play(const uint16_t *buffer, int length);

    /**
     * Interrupt callback when playback of DMA buffer has completed
     */
    virtual void dmaTransferComplete();
};

#endif
