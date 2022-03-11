#include "MicroBit.h"
#include "Tests.h"

MicroBit uBit;

int 
main()
{
    uBit.init();

    while(1) {
        rec_test_main();

        uBit.sleep( 1000  );
    }
}