#include "MicroBit.h"

extern MicroBit uBit;

void rec_simple_recorder()
{
    uBit.audio.activateMic();
    uBit.sleep( 1000 );
    uBit.audio.deactivateMic();
    uBit.sleep( 1000 );
}

void rec_test_main()
{
    rec_simple_recorder();
}