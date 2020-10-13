#include "MicroBit.h"
#include "Tests.h"

void
onCompassData(MicroBitEvent)
{
    DMESGN("C");
}

void
onAccelerometerData(MicroBitEvent)
{
    DMESGN("A");
}

void
onShake(MicroBitEvent)
{
    DMESG(" *** SHAKE ***");
    uBit.display.print("S");
    uBit.sleep(500);
    uBit.display.clear();
}



void
accelerometer_test1()
{
    while(1)
    {
        DMESG("Acc [X:%d][Y:%d][Z:%d]", uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ());
        uBit.sleep(1000);
    }
}

int g_to_pix(int g)
{
    int v = 2;
    if ( g < -200) v--;
    if ( g < -500) v--;
    if ( g > 200) v++;
    if ( g > 500) v++;

    return v;
}    

void
spirit_level2()
{
    DMESG("SPIRIT_LEVEL_2");

    int ox=0;
    int oy=0;
    int moves = 0;

    while(moves < 15)
    {
        int x = uBit.accelerometer.getX();
        int y = uBit.accelerometer.getY();
        int z = uBit.accelerometer.getZ();

        DMESG("Acc [X:%d][Y:%d][Z:%d]\r\n", x, y, z);

        int px = g_to_pix(x);
        int py = g_to_pix(y);

        if (ox != px || oy != py)
            moves++;

        ox = px;
        oy = py;

        uBit.display.image.clear();
        uBit.display.image.setPixelValue(px,py,255);

        uBit.sleep(100);
    }

    uBit.display.clear();
}

void
spirit_level()
{
    while(1)
    {
        int x = uBit.accelerometer.getX();
        int y = uBit.accelerometer.getY();
        int z = uBit.accelerometer.getZ();

        DMESG("Acc [X:%d][Y:%d][Z:%d]", x, y, z);

        int px = g_to_pix(x);
        int py = g_to_pix(y);

        uBit.display.image.clear();
        uBit.display.image.setPixelValue(px,py,255);

        uBit.sleep(100);
    }
}

void
compass_test1()
{
    while(1)
    {
        DMESG("Mag [X:%d][Y:%d][Z:%d]", uBit.compass.getX(), uBit.compass.getY(), uBit.compass.getZ());
        uBit.sleep(1000);
    }
}

void
compass_test2()
{
    while(1)
    {
        DMESG("Heading [%d]", uBit.compass.heading());
        uBit.sleep(1000);
    }
}

void
compass_accelerometer_test()
{
    while(1)
    {
        DMESG("ACC [X:%d][Y:%d][Z:%d]\n", uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ());
        DMESG("MAG [X:%d][Y:%d][Z:%d]\n", uBit.compass.getX(), uBit.compass.getY(), uBit.compass.getZ());

        uBit.sleep(1000);
    }    
}

void
shake_test()
{
    //uBit.messageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_EVT_DATA_UPDATE, onAccelerometerData);
    //uBit.messageBus.listen(MICROBIT_ID_COMPASS, MICROBIT_COMPASS_EVT_DATA_UPDATE, onCompassData);
    uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_ACCELEROMETER_EVT_SHAKE, onShake);

    while(1)
    {
        uBit.sleep(10000);
    }

}
