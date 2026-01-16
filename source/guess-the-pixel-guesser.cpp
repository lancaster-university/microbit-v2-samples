#include "MicroBit.h"

MicroBit uBit;

int main ( ) {
    uBit.init();
    while (true) {
        uBit.display.scroll("Hello World, Micro Edition");
    }
}