#include "MicroBit.h"

extern MicroBit uBit;

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
