#include "MicroBit.h"
#include "samples/Tests.h"

MicroBit uBit;

int main()
{
    uBit.init();

    //out_of_box_experience();
    mems_record_playback_test();

    microbit_panic( 911 );
}