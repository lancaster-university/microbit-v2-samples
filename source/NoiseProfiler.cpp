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

#include "NoiseProfiler.h"
#include "Tests.h"

/**
* Creates a simple component that generates a noise profile of an 8 bit data stream provided
* @param source a DataSource to measure.
*/
NoiseProfiler::NoiseProfiler(DataSource &source) : upstream(source)
{
    reset();

    // Register with our upstream component
    source.connect(*this);
}

/**
 * Callback provided when data is ready.
 */
int 
NoiseProfiler::pullRequest()
{
    ManagedBuffer buf = upstream.pull();

    int8_t *p = (int8_t *)&buf[0];
    int8_t *end = (int8_t *)p + buf.length();
    int s,s1;
    bool first = true;

    while (p < end)
    {
        if (samples < NOISE_PROFILE_TOTAL_SAMPLES)
        {
            s = *p;

            if (s <= NOISE_PROFILE_RANGE && s >= -NOISE_PROFILE_RANGE)
                noiseProfile[s+NOISE_PROFILE_RANGE]++;

            if (first)
                first = false;
            else
                variance += abs(s-s1);
                         
            s1 = s;
            samples++;

        }

        p++;
    }

    return DEVICE_OK;
}

void 
NoiseProfiler::reset()
{
    // Reset our state
    for (int i=0; i<NOISE_PROFILE_SIZE; i++)
        noiseProfile[i] = 0;

    samples = 0;
    variance = 0;
}

/**
* Output the results gathered to the DMESG buffer
*/
void 
NoiseProfiler::printResults()
{
    DMESG("NOISE_PROFILE:");
    DMESG("   SAMPLES: %d", samples);
    DMESG("   VARIANCE: %d", variance);


    for (int i=0; i<NOISE_PROFILE_SIZE; i++)
        DMESG("   LEVEL [%d]: %d", i-NOISE_PROFILE_RANGE, noiseProfile[i]);
}

/**
* Determines the state of the test
* @return true if the test is complete, fasle otherwise
*/
bool 
NoiseProfiler::isDone()
{
    return samples >= NOISE_PROFILE_TOTAL_SAMPLES;
}