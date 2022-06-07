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

#ifndef NOISE_PROFILER_H
#define NOISE_PROFILER_H

#define NOISE_PROFILE_RANGE 10
#define NOISE_PROFILE_SIZE (2*NOISE_PROFILE_RANGE+1)
#define NOISE_PROFILE_TOTAL_SAMPLES 110000


class NoiseProfiler : public DataSink
{
    DataSource      &upstream;          
    int             noiseProfile[NOISE_PROFILE_SIZE];
    int             variance;
    int             samples;

    public:
    /**
     * Creates a simple component that generates a noise profile of an 8 bit data stream provided
     * @param source a DataSource to measure.
     */
    NoiseProfiler(DataSource &source);

    /**
     * Callback provided when data is ready.
     */
    virtual int pullRequest();

    /**
    * reset gathered data
    */
    void reset();

    /**
    * Output the results gathered to the DMESG buffer
    */
    void printResults();

    /**
    * Determines the state of the test
    * @return true if the test is complete, fasle otherwise
    */
    bool isDone();
};

#endif
