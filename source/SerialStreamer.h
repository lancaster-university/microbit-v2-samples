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

#include "Microbit.h"
#include "CodalConfig.h"
#include "DataStream.h"

#ifndef SERIAL_STREAMER_H
#define SERIAL_STREAMER_H

#define SERIAL_STREAM_MODE_LINEAR_8BIT          1
#define SERIAL_STREAM_MODE_OFFSET_AND_SCALE     2

class SerialStreamer : public DataSink
{
    DataSource      &upstream;          
    int             zeroOffset;             // unsigned value that is the best effort guess of the zero point of the data source
    int             divisor;                // Used for LINEAR modes
    int             mode;

    public:
    /**
     * Creates a simple component that logs a stream of signed 16 bit data as signed 8-bit data over serial.
     * @param source a DataSource to measure the level of.
     */
    SerialStreamer(DataSource &source);

    /**
     * Callback provided when data is ready.
     */
    virtual int pullRequest();

    /**
     * Sets the attentuation to apply as samples are translated from 16 bit to 8 bit
     */
    int setDivisor(int d);

};

#endif
