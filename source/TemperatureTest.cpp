#include "MicroBit.h"

extern MicroBit uBit;

void
temperature_test()
{
    while(1)
    {
        uBit.sleep(1000);
        uBit.display.scroll(uBit.thermometer.getTemperature());
    }
}

