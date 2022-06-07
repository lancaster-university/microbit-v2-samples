#include "MicroBit.h"
#include "Tests.h"

int clicks;
int clickmode;

void red_power_test()
{
    uBit.io.row1.setDigitalValue(1);
    uBit.io.row2.getDigitalValue();
    uBit.io.row3.getDigitalValue();
    uBit.io.row4.getDigitalValue();
    uBit.io.row5.getDigitalValue();

    uBit.io.col1.setDigitalValue(0);
    uBit.io.col2.getDigitalValue();
    uBit.io.col3.getDigitalValue();
    uBit.io.col4.getDigitalValue();
    uBit.io.col5.getDigitalValue();

}

void green_power_test()
{
    uBit.io.row1.setDigitalValue(0);
    uBit.io.row2.getDigitalValue();
    uBit.io.row3.getDigitalValue();
    uBit.io.row4.getDigitalValue();
    uBit.io.row5.getDigitalValue();

    uBit.io.col1.setDigitalValue(1);
    uBit.io.col2.getDigitalValue();
    uBit.io.col3.getDigitalValue();
    uBit.io.col4.getDigitalValue();
    uBit.io.col5.getDigitalValue();

}

void off_power_test()
{
    uBit.io.row1.getDigitalValue();
    uBit.io.row2.getDigitalValue();
    uBit.io.row3.getDigitalValue();
    uBit.io.row4.getDigitalValue();
    uBit.io.row5.getDigitalValue();

    uBit.io.col1.getDigitalValue();
    uBit.io.col2.getDigitalValue();
    uBit.io.col3.getDigitalValue();
    uBit.io.col4.getDigitalValue();
    uBit.io.col5.getDigitalValue();

}


void setDisplay(int mode)
{
    DMESG(mode ? "RED\n" : "GREEN\n");

    uBit.io.row1.setDigitalValue(mode);
    uBit.io.row2.setDigitalValue(mode);
    uBit.io.row3.setDigitalValue(mode);
    uBit.io.row4.setDigitalValue(mode);
    uBit.io.row5.setDigitalValue(mode);

    uBit.io.col1.setDigitalValue(!mode);
    uBit.io.col2.setDigitalValue(!mode);
    uBit.io.col3.setDigitalValue(!mode);
    uBit.io.col4.setDigitalValue(!mode);
    uBit.io.col5.setDigitalValue(!mode);
}

void setCol(int col, int mode)
{
    int c = 0;

    for (NRF52Pin *p : uBit.ledRowPins)
        p->setDigitalValue(mode == 2 ? 0 : mode);

    for (NRF52Pin *p : uBit.ledColPins)
    {
        if (c == col)
            p->setDigitalValue(mode == 2 ? 0 : !mode);
        else
            p->setDigitalValue(mode == 2 ? 0 : mode);

        c++;
    }
}


static void onButtonA(MicroBitEvent)
{
    if (clickmode != 1)
        clicks++;

    clickmode = 1;

    setDisplay(1);
}

static void onButtonB(MicroBitEvent)
{
    if (clickmode != 2)
        clicks++;

    clickmode = 2;

    setDisplay(0);
}

void button_blinky_test()
{
    clicks = 0;
    clickmode = 0;
    uBit.display.disable();
    uBit.messageBus.listen(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonB);    

    setDisplay(1);

    release_fiber();
}

void button_blinky_test2()
{
    clicks = 0;
    clickmode = 0;
    uBit.display.disable();
    uBit.messageBus.listen(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonB);    

    setDisplay(1);

    while(clicks < 4)
        uBit.sleep(100);

    uBit.messageBus.ignore(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.ignore(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonB);    
    uBit.display.enable();
}

void fade_test()
{
    uBit.display.disable();

    int period = 40;
    int brightness[5] = {0,20,40,60,80};
    int dx[5] = {1,1,1,1,1};
    int compression = 25;

    while(1)
    {
        for (int c=0; c<5; c++)
        {
            //green
            setCol(c==2?5:c,0);
            target_wait_us(period * brightness[c]) ;

            //red
            setCol(c,1);
            target_wait_us(period * (((100-brightness[c])*compression)/100));
        }

        for (int c=0; c<5; c++)
        {
            brightness[c] += dx[c];

            if (brightness[c] == 100 || brightness[c] == 0)
                dx[c] = -dx[c];
        }
    }

}

void blinky() 
{
    uBit.display.disable();
    uBit.io.row1.setDigitalValue(1);
    uBit.io.row2.setDigitalValue(1);

    while (1) {

        uBit.io.col1.setDigitalValue(0);
        uBit.io.col2.setDigitalValue(0);

        DMESG("ON");
        uBit.sleep(500);

        uBit.io.col1.setDigitalValue(1);
        uBit.io.col2.setDigitalValue(1);

        DMESG("OFF");
        uBit.sleep(500);
    }
}


