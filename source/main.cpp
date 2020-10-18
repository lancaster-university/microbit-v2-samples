#include "MicroBit.h"
#include "Tests.h"

MicroBit uBit;

int 
main()
{
    uBit.init();

    audio_virtual_pin_melody();
}