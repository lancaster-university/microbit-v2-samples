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

#include "MicroBit.h"
#include "CodalConfig.h"
#include "DataStream.h"

#ifndef SERIAL_STREAMER_H
#define SERIAL_STREAMER_H

#define SERIAL_STREAM_MODE_BINARY               1
#define SERIAL_STREAM_MODE_DECIMAL              2
#define SERIAL_STREAM_MODE_HEX                  4

class SerialStreamer : public DataSink
{
    DataSource      &upstream; 
    ManagedBuffer   lastBuffer;         
    int             mode;

    public:
    /**
     * Creates a simple component that logs a stream of signed 16 bit data as signed 8-bit data over serial.
     * @param source a DataSource to measure the level of.
     * @param mode the format of the serialised data. Valid options are SERIAL_STREAM_MODE_BINARY (default), SERIAL_STREAM_MODE_DECIMAL, SERIAL_STREAM_MODE_HEX.
     */
    SerialStreamer(DataSource &source, int mode = SERIAL_STREAM_MODE_BINARY);

    /**
     * Callback provided when data is ready.
     */
    virtual int pullRequest();

    /**
     * Stream the last buffer received to the serial port.
     * n.b. this occurs automatically upon the buffer is made available by our upstream component.
     * Call this method explicitly only if your wish to send the buffer again.
     */
    void streamBuffer(ManagedBuffer buffer);

    /**
     * returns the last buffer processed by this component
     */
    ManagedBuffer getLastBuffer();
};

#endif
