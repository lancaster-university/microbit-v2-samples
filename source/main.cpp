#include "MicroBit.h"
#include "neopixel.h"

MicroBit uBit;

#define NUM_OF_NEOPIXELS 256

int 
main()
{
    uBit.init();
    uint8_t buffer[3 * NUM_OF_NEOPIXELS];

    uBit.io.P1.setDigitalValue(1);
    uBit.io.P0.setHighDrive(true);

    for(int i = 0; i < NUM_OF_NEOPIXELS; i++) {
        buffer[3*i    ] = 0xff;
        buffer[3*i + 1] = 0x00;
        buffer[3*i + 2] = 0x00;
    }

    neopixel_send_buffer(uBit.io.P0, buffer, NUM_OF_NEOPIXELS * 3);

    while(1) {
        uBit.display.scroll("HELLO WORLD!");
    }
}
