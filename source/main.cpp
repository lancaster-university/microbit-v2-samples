#include "MicroBit.h"

MicroBit uBit;

int 
main()
{
    uBit.init();

    while(1)
        uBit.display.scroll("HELLO WORLD!");
}