#include "MicroBit.h"
#include "Tests.h"

const char * const arrow_left_emoji ="\
    000,000,255,000,000\n\
    000,255,000,000,000\n\
    255,255,255,255,255\n\
    000,255,000,000,000\n\
    000,000,255,000,000\n";
const char * const arrow_right_emoji ="\
    000,000,255,000,000\n\
    000,000,000,255,000\n\
    255,255,255,255,255\n\
    000,000,000,255,000\n\
    000,000,255,000,000\n";
const char * const tick_emoji ="\
    000,000,000,000,000\n\
    000,000,000,000,255\n\
    000,000,000,255,000\n\
    255,000,255,000,000\n\
    000,255,000,000,000\n";

const char * const radio_emoji ="\
    255,255,255,000,000\n\
    000,000,000,255,000\n\
    255,255,000,000,255\n\
    000,000,255,000,255\n\
    255,000,255,000,255\n";

const char * const happy_emoji ="\
    000,255,000,255,000\n\
    000,000,000,000,000\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

const char * const wink_emoji ="\
    000,255,000,000,000\n\
    000,000,000,000,000\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

const char * const sad_emoji ="\
    000,255,000,255,000\n\
    000,000,000,000,000\n\
    000,000,000,000,000\n\
    000,255,255,255,000\n\
    255,000,000,000,255\n";

Image happy(happy_emoji);
Image sad(sad_emoji);

void
concurrent_display_test_t1()
{
    while(1)
    {
        uBit.display.print("JJ");
        uBit.sleep(1000);
    }
}

void
concurrent_display_test_t2()
{
    uBit.sleep(500);
    while(1)
    {
        uBit.display.print("FF");
        uBit.sleep(1000);
    }
}

void
concurrent_display_test()
{
    DMESG("CONCURRENT_DISPLAY_TEST1:");

    create_fiber(concurrent_display_test_t1);
    create_fiber(concurrent_display_test_t2);

    release_fiber();
}

void
display_test1()
{
    DMESG("DISPLAY_TEST1:");
    while(1)
    {
        uBit.display.image.print('J');
        uBit.sleep(500);
        uBit.display.image.print('F');
        uBit.sleep(500);
    }
}

void
display_test2()
{
    DMESG("DISPLAY_TEST2:");
    while(1)
    {
        uBit.display.scroll("HELLO");
        uBit.display.scroll("WORLD");
        uBit.sleep(2000);
    }
}

void
display_wink()
{
    DMESG("DISPLAY_WINK:");

    MicroBitImage smile(happy_emoji);
    MicroBitImage wink(wink_emoji);

    uBit.display.print(smile);
    uBit.sleep(1000);
    uBit.display.print(wink);
    uBit.sleep(1000);
    uBit.display.print(smile);
    uBit.sleep(1000);

    uBit.display.clear();
}

void
display_brightness_test()
{
    MicroBitImage smile(happy_emoji);
    uBit.display.print(smile);
    uBit.display.setBrightness(50);

    while(1)
    {
        for (int i=0; i<=255; i++)
        {
            uBit.display.setBrightness(i);
            uBit.sleep(10);
        }

        for (int i=255; i>=0; i--)
        {
            uBit.display.setBrightness(i);
            uBit.sleep(10);
        }
    }
}


void
display_tick()
{
    DMESG("DISPLAY_TICK:");

    MicroBitImage tick(tick_emoji);
    uBit.display.print(tick);
}

void
display_arrows()
{
    DMESG("DISPLAY_ARROWS:");

    MicroBitImage arrowL(arrow_left_emoji);
    MicroBitImage arrowR(arrow_right_emoji);

    uBit.display.print(arrowL);
    uBit.sleep(200);
    uBit.display.clear();
    uBit.sleep(200);
    uBit.display.print(arrowL);
    uBit.sleep(200);
    uBit.display.clear();
    uBit.sleep(200);

    uBit.display.print(arrowR);
    uBit.sleep(200);
    uBit.display.clear();
    uBit.sleep(200);
    uBit.display.print(arrowR);
    uBit.sleep(200);
    uBit.display.clear();
    uBit.sleep(200);

    uBit.display.clear();
}

void
display_radio()
{
    DMESG("DISPLAY_RADIO:");

    MicroBitImage radio(radio_emoji);
    uBit.display.print(radio);
}

void
display_countdown()
{
    for (int i=9; i>0; i--)
    {
        uBit.display.print(i);
        uBit.sleep(1000);
    }
}

void
raw_blinky_test()
{
    uBit.io.row1.setDigitalValue(0);
    uBit.io.row2.setDigitalValue(0);
    uBit.io.row3.setDigitalValue(0);
    uBit.io.row4.setDigitalValue(0);
    uBit.io.row5.setDigitalValue(0);

    uBit.io.col1.setDigitalValue(0);
    uBit.io.col2.setDigitalValue(0);
    uBit.io.col3.setDigitalValue(0);
    uBit.io.col4.setDigitalValue(0);
    uBit.io.col5.setDigitalValue(0);

    while(1)
    {
        uBit.io.row1.setDigitalValue(1);
        uBit.io.row2.setDigitalValue(1);
        uBit.io.row3.setDigitalValue(1);
        uBit.io.row4.setDigitalValue(1);
        uBit.io.row5.setDigitalValue(1);
        DMESG("LED: ON...\n");
        uBit.sleep(500);

        uBit.io.row1.setDigitalValue(0);
        uBit.io.row2.setDigitalValue(0);
        uBit.io.row3.setDigitalValue(0);
        uBit.io.row4.setDigitalValue(0);
        uBit.io.row5.setDigitalValue(0);
        DMESG("LED: OFF...\n");
        uBit.sleep(500);
    }
}


void
onButtonAPressed(MicroBitEvent)
{
    const char * const a_emoji ="\
        255,000,000,000,255\n\
        000,000,000,000,000\n\
        255,255,255,255,255\n\
        000,000,000,255,255\n\
        000,000,000,255,255\n";

    MicroBitImage img_a(a_emoji);
    uBit.display.print(img_a);
}

void
onButtonBPressed(MicroBitEvent)
{
    const char * const b_emoji ="\
        000,255,000,000,255\n\
        000,000,255,255,000\n\
        000,000,255,255,000\n\
        000,000,255,255,000\n\
        000,255,000,000,255\n";
    MicroBitImage img_b(b_emoji);
    uBit.display.print(img_b);    
}

void
onButtonABPressed(MicroBitEvent)
{
    const char * const c_emoji ="\
        000,000,000,255,255\n\
        000,000,000,255,255\n\
        255,255,255,255,255\n\
        255,255,255,255,255\n\
        000,255,000,255,000\n";
    MicroBitImage img_c(c_emoji);
    uBit.display.print(img_c);    
}

void
onShakePressed(MicroBitEvent)
{
    const char * const d_emoji ="\
        255,000,000,000,255\n\
        000,255,000,255,000\n\
        000,000,000,000,000\n\
        255,255,255,255,255\n\
        255,000,255,000,255\n";
    MicroBitImage img_d(d_emoji);
    uBit.display.print(img_d);    
}

void
do_something_forever()
{
    uBit.sleep(10);
}

void 
display_button_icon_test()
{
    const char * const heart_emoji ="\
    000,255,000,255,000\n\
    255,255,255,255,255\n\
    255,255,255,255,255\n\
    000,255,255,255,000\n\
    000,000,255,000,000\n";

    MicroBitImage img_heart(heart_emoji);
    uBit.display.print(img_heart);

    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonAPressed);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonBPressed);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_CLICK, onButtonABPressed);
    uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_ACCELEROMETER_EVT_SHAKE, onShakePressed);

    create_fiber(do_something_forever);
    release_fiber();
}

