#include "DeviceConfig.h"
#include "SystemClock.h"
#include "DevicePin.h"
#include "SAMD21DMAC.h"

#ifndef SAMD21DAC_H
#define SAMD21DAC_H

class SAMD21DAC : DeviceComponent
{

private:
    SAMD21DMAC  &dmac;
    int         dmaChannel;

public:
    
    /**
      * Constructor for an instance of a DAC,
      *
      * @param pin The pin this DAC shoudl output to.
      * @param id The id to use for the message bus when transmitting events.
      */
    SAMD21DAC(DevicePin &pin, SAMD21DMAC &dma, uint16_t id = DEVICE_ID_SYSTEM_DAC);

    void setValue(int value);
    int getValue();
    void play(uint16_t *buffer, int length);
};

#endif
