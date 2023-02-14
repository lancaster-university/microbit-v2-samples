/*
The MIT License (MIT)

Copyright (c) 2021 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "MicroBit.h"
#include "Tests.h"
#include "neopixel.h"

extern MicroBit uBit;

void
neopixel_test()
{
    // Length of neopixel strip
    int length = 8;

    // Brightness of pixels
    int brightness = 32;

    // Colour counter variable (just used to remember what colour we last displayed)
    int c = 0;

    // Create a buffer to hold pixel data of the appropriate size
    ManagedBuffer b(length*3);

    while(1)
    {
        // Clear the buffer to zeroes. You don't need to do this, but it's often convenient.
        b.fill(0);

        // Fill the buffer with some data.
        // Neopixel typically has 3 bytes per pixel, for each primary colour
        // in GRB format (Green, then Red, then Blue).
        for (int i=0; i<length*3; i+=3)
            b[i+c] = brightness;

        // Update the neopixel strip on pin P0.
        neopixel_send_buffer(uBit.io.P0, b);

        // Increment our colour counter so that we show a different colour in the next frame.
        c = (c+1) %3;

        // Wait for a second.
        uBit.sleep(1000);
    }
}
