#include "MicroBit.h"
#include "Tests.h"

void
temperature_test()
{
    while(1)
    {
        DMESG("TEMPERATURE: %d", uBit.thermometer.getTemperature());
        uBit.display.scroll(uBit.thermometer.getTemperature());
    }
}

