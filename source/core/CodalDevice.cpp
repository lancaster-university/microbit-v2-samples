/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

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

#include "CodalDevice.h"
#include "CodalDmesg.h"

void CodalDevice::panic(int statusCode)
{
    this->disableInterrupts();

#if DEVICE_DMESG_BUFFER_SIZE > 0
    DMESG("*** CODAL PANIC : [%d]", statusCode);
    while (1)
    {
    }
#else
    Serial pc(USBTX, USBRX);
    while (1)
    {
        pc.printf("*** CODAL PANIC : [%.3d]\n", statusCode);
        wait_ms(500);
    }
#endif
}

int CodalDevice::random(int max)
{
    uint32_t m, result;

    if (max <= 0)
        return DEVICE_INVALID_PARAMETER;

    // Our maximum return value is actually one less than passed
    max--;

    do
    {
        m = (uint32_t)max;
        result = 0;
        do
        {
            // Cycle the LFSR (Linear Feedback Shift Register).
            // We use an optimal sequence with a period of 2^32-1, as defined by Bruce Schneier here
            // (a true legend in the field!),
            // For those interested, it's documented in his paper:
            // "Pseudo-Random Sequence Generator for 32-Bit CPUs: A fast, machine-independent
            // generator for 32-bit Microprocessors"
            // https://www.schneier.com/paper-pseudorandom-sequence.html
            uint32_t rnd = random_value;

            rnd = ((((rnd >> 31) ^ (rnd >> 6) ^ (rnd >> 4) ^ (rnd >> 2) ^ (rnd >> 1) ^ rnd) &
                    0x0000001)
                   << 31) |
                  (rnd >> 1);

            random_value = rnd;

            result = ((result << 1) | (rnd & 0x00000001));
        } while (m >>= 1);
    } while (result > (uint32_t)max);

    return result;
}
