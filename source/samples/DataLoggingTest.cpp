#include "MicroBit.h"
#include "Tests.h"

void data_logging_timeseries()
{
    uBit.log.setTimeStamp(TimeStampFormat::Seconds);
    bool logging = false;
    while (1)
    {
        if (uBit.buttonA.wasPressed())
        {
            uBit.display.scroll("L");
            logging = true;
        }
        if (uBit.buttonB.wasPressed())
        {
            uBit.display.scroll("C");
            logging = false;
            uBit.log.clear(false);
        }
        if (uBit.log.isFull())
        {
            uBit.display.scroll("F");
            logging = false;
        }
        if (logging)
        {
            uBit.log.beginRow();
            uBit.log.logData("x", uBit.accelerometer.getX());
            uBit.log.logData("y", uBit.accelerometer.getY());
            uBit.log.logData("z", uBit.accelerometer.getZ());
            uBit.log.endRow();
            uBit.sleep(250);
        }
    }
}