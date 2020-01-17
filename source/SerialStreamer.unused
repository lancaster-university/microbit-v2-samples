/*
The MIT License (MIT)

Copyright (c) 2016 Lancaster University.

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

#include "SerialStreamer.h"
#include "Tests.h"

SerialStreamer::SerialStreamer(DataSource &source) : upstream(source)
{
    // Register with our upstream component
    source.connect(*this);

    zeroOffset = 0;
    mode = SERIAL_STREAM_MODE_OFFSET_AND_SCALE;
}

/**
 * Callback provided when data is ready.
 */
int SerialStreamer::pullRequest()
{
    int z = 0;
    int minimum = 0;
    int maximum = 0;
    int s;
    int8_t result;

    ManagedBuffer buf = upstream.pull();

    int16_t *data = (int16_t *) &buf[0];
    int samples = buf.length() / 2;

    for (int i=0; i < samples; i++)
    {
        z += *data;
        if (mode == SERIAL_STREAM_MODE_LINEAR_8BIT)
        {
            result = (int8_t) (*data / divisor);
            data++;

            uBit.serial.putc(result);
        }

        if (mode == SERIAL_STREAM_MODE_OFFSET_AND_SCALE)
        {
            s = (int) *data;
            s = s - zeroOffset;
            s = s / divisor;
            result = (int8_t)s;

            //s = s + range;
            //s = s * gain;
            //s = s >> 10;
            //s = s - range;

            if (s < minimum)
                minimum = s;

            if (s > maximum)
                maximum = s;

            data++;

            uBit.serial.putc(result);
        }
    }

    z = z / samples;
    zeroOffset = z;

    //DMESG("[Z: %d] [R: %d]", zeroOffset, maximum - minimum);

    return DEVICE_OK;
}

int
SerialStreamer::setDivisor(int d)
{
    divisor = d;
    return DEVICE_OK;
}


