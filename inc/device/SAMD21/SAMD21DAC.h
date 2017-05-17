#include "DeviceConfig.h"
#include "SystemClock.h"
#include "DevicePin.h"
#include "SAMD21DMAC.h"
#include "DataStream.h"

#ifndef SAMD21DAC_H
#define SAMD21DAC_H

#ifndef SAMD21DAC_DEFAULT_FREQUENCY
#define SAMD21DAC_DEFAULT_FREQUENCY 44100
#endif

class SAMD21DAC : public DeviceComponent, public DmaComponent, public DataSink
{

private:
    SAMD21DMAC  &dmac;
    int         dmaChannel;
    bool        active;
    int         dataReady;
    int         sampleRate;

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
    SAMD21DAC(DevicePin &pin, SAMD21DMAC &dma, DataSource &source, int sampleRate = SAMD21DAC_DEFAULT_FREQUENCY, uint16_t id = DEVICE_ID_SYSTEM_DAC);

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
     * Change the DAC playback sample rate to the given frequency.
     * n.b. Only sample periods that are a multiple of 125nS are supported. 
     * Frequencies mathcing other sample periods will be rounded down to the next lowest supported frequency.
     *
     * @param frequency The new sample playback frequency.
     */
    int getSampleRate();

    /**
     * Change the DAC playback sample rate to the given frequency.
     * n.b. Only sample periods that are a multiple of 125nS are supported. 
     * Frequencies mathcing other sample periods will be rounded to the next highest supported frequency.
     *
     * @param frequency The new sample playback frequency.
     */
    int setSampleRate(int frequency);

    /**
     * Interrupt callback when playback of DMA buffer has completed
     */
    virtual void dmaTransferComplete();
};

#endif
