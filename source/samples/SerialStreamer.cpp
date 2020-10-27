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

/**
 * Creates a simple component that logs a stream of signed 16 bit data as signed 8-bit data over serial.
 * @param source a DataSource to measure the level of.
 * @param mode the format of the serialised data. Valid options are SERIAL_STREAM_MODE_BINARY (default), SERIAL_STREAM_MODE_DECIMAL, SERIAL_STREAM_MODE_HEX.
 */
SerialStreamer::SerialStreamer(DataSource &source, int mode) : upstream(source)
{
    this->mode = mode;

    // Register with our upstream component
    source.connect(*this);
}

/**
 * Callback provided when data is ready.
 */
int SerialStreamer::pullRequest()
{
    static volatile int pr = 0;
     
    if(!pr)
    {
        pr++;
        while(pr)
        {
            lastBuffer = upstream.pull();
            streamBuffer(lastBuffer);
            pr--;
        }
    }
    else
    {
        pr++;
    }
    
    return DEVICE_OK;
}

/**
    * returns the last buffer processed by this component
    */
ManagedBuffer SerialStreamer::getLastBuffer()
{
    return lastBuffer;
}

/**
 * Callback provided when data is ready.
 */
void SerialStreamer::streamBuffer(ManagedBuffer buffer)
{
    int CRLF = 0;
    int bps = upstream.getFormat();

    // If a BINARY mode is requested, simply output all the bytes to the serial port.
    if (mode == SERIAL_STREAM_MODE_BINARY)
    {
        uint8_t *p = &buffer[0];
        uint8_t *end = p + buffer.length();

        while(p < end)
            uBit.serial.putc(*p++);
    }

    // if a HEX mode is requested, format using printf, framed by sample size..
    if (mode == SERIAL_STREAM_MODE_HEX  || mode == SERIAL_STREAM_MODE_DECIMAL)
    {
        uint8_t *d = &buffer[0];
        uint8_t *end = d+buffer.length();
        uint32_t data;

        while(d < end)
        {
            data = *d++;

            if (bps > DATASTREAM_FORMAT_8BIT_SIGNED)
                data |= (*d++) << 8;
            if (bps > DATASTREAM_FORMAT_16BIT_SIGNED)
                data |= (*d++) << 16;
            if (bps > DATASTREAM_FORMAT_24BIT_SIGNED)
                data |= (*d++) << 24;

            if (mode == SERIAL_STREAM_MODE_HEX)
                uBit.serial.printf("%x ", data);
            else
                uBit.serial.printf("%d ", data);

            CRLF++;

            if (CRLF == 16){
                uBit.serial.printf("\n");
                CRLF = 0;
            }
        }
        
        if (CRLF > 0)
            uBit.serial.printf("\n");
    }

    // We're alway hungry, so deschedule ourselves after processing each buffer.
    schedule();
}


