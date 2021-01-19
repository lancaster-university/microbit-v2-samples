#include "MicroBit.h"
#include "Tests.h"

MicroBit uBit;

int 
main()
{
    uBit.init();
    ble_test();

    while(1)
        uBit.display.scroll("HELLO WORLD!");
}
