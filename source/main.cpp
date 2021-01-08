#include "MicroBit.h"
#include "neopixel.h"

MicroBit uBit;

int 
main()
{
    uBit.init();
    uint8_t buffer[24] = {
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,
    };

    uBit.io.P1.setDigitalValue(1);

    uBit.io.P0.setHighDrive(true);
    neopixel_send_buffer(uBit.io.P0, buffer, 24);

    while(1) {
        uBit.display.scroll("HELLO WORLD!");
    }
}
