#include "MicroBit.h"
extern MicroBit uBit;

Pin *edgeConnector[] = {
    &uBit.io.P0, 
    &uBit.io.P1, 
    &uBit.io.P2, 
    &uBit.io.P3, 
    &uBit.io.P4, 
    &uBit.io.P5, 
    &uBit.io.P6, 
    &uBit.io.P7, 
    &uBit.io.P8, 
    &uBit.io.P9, 
    &uBit.io.P10, 
    &uBit.io.P11, 
    &uBit.io.P12, 
    &uBit.io.P13, 
    &uBit.io.P14, 
    &uBit.io.P15, 
    &uBit.io.P16, 
    &uBit.io.P19, 
    &uBit.io.P20
};

//Pin *analogPins[] = {&uBit.io.P1, &uBit.io.P2};
Pin *analogPins[] = {&uBit.io.P1};

void
edge_connector_test()
{
    uBit.display.disable();
    uBit.buttonA.disable();
    uBit.buttonB.disable();

    for (Pin *p : edgeConnector)
        p->setDigitalValue(0);

    while(1)
    {
        for (Pin *p : edgeConnector)
        {
            p->setDigitalValue(1);
            uBit.sleep(500);
            p->setDigitalValue(0);
        }
    }
}

void
analog_test()
{
    Image level(5,5);

    while(1)
    {
        int x=0;
        int y=0; 
        int px = analogPins[0]->getAnalogValue() / 40;

        level.clear();

        while(px)
        {
            level.setPixelValue(x,y, 255);
            x = (x+1) % 5;
            if (x == 0)
                y = (y+1) % 5;

            px--;
        }

        uBit.display.print(level);

        for (Pin *p : analogPins)
        {
            int v = p->getAnalogValue();
            DMESG("%d", v);
        }

        DMESG("----");
        uBit.sleep(100);
    }
}
void
gpio_test()
{
    int value = 0;

    uBit.io.P2.getDigitalValue();
    uBit.io.P2.setPull(PullUp);

    while(1)
    {
        if (uBit.io.P2.getDigitalValue())
            uBit.display.image.setPixelValue(0,0,255);
        else
            uBit.display.image.setPixelValue(0,0,0);

        uBit.io.P1.setDigitalValue(value);
        value = !value;

        uBit.sleep(100);
    }
}

