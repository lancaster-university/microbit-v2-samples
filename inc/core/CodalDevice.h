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

#ifndef CODAL_DEVICE_H
#define CODAL_DEVICE_H

#include "DeviceConfig.h"
#include "ErrorNo.h"

/**
  * Class definition for CodalDevice.
  *
  * All devices should inherit from this class, and override any of the methods below
  * to provide define core functionality needed by codal. 
  *
  */
class CodalDevice
{
    public:
    uint32_t random_value;

    /**
      * The default constructor of a DeviceComponent
      */
    CodalDevice()
    {
    }

    /**
      * Perform a hard reset of the device.
      * default: Use CMSIS NVIC reset vector.
      */
    virtual void reset()
    {
        NVIC_SystemReset();
    }

    /**
     * Determine the version of system software currently running.
     * default: The version of the codal runtime being used.
     * @return a pointer to a NULL terminated character buffer containing a representation of the current version using semantic versioning.
     */
    virtual const char *
    device_dal_version()
    {
        return DEVICE_DAL_VERSION;
    }

    /**
      * Determines a unique 32 bit ID for this device, if provided by the hardware. 
      * default: 0.
      * @return A 32 bit unique identifier.
      */
    virtual uint32_t getSerialNumber()
    {
        return 0;
    }

    /**
     * Default: Disables all interrupts and user processing and periodically outputs the status code over the default USB serial port.
     * @param statusCode the appropriate status code, must be in the range 0-999.
     */
    virtual void panic(int statusCode)
    {
        __disable_irq(); 

        Serial pc(USBTX, USBRX); 
        while(1) 
        {
            pc.printf("*** CODAL PANIC : [%.3d]\n", statusCode);
            wait_ms(500);
        }
    }

    /**
     * Generate a random number in the given range.
     * default: A simple Galois LFSR random number generator.
     * A well seeded Galois LFSR is sufficient for most applications, and much more lightweight
     * than hardware random number generators often built int processor, which takes
     * a long time and uses a lot of energy.
     *
     * @param max the upper range to generate a number for. This number cannot be negative.
     * @return A random, natural number between 0 and the max-1. Or DEVICE_INVALID_VALUE if max is <= 0.
     */
    int random(int max)
    {
        uint32_t m, result;

        if(max <= 0)
            return DEVICE_INVALID_PARAMETER;

        // Our maximum return value is actually one less than passed
        max--;

        do {
            m = (uint32_t)max;
            result = 0;
            do {
                // Cycle the LFSR (Linear Feedback Shift Register).
                // We use an optimal sequence with a period of 2^32-1, as defined by Bruce Schneier here (a true legend in the field!),
                // For those interested, it's documented in his paper:
                // "Pseudo-Random Sequence Generator for 32-Bit CPUs: A fast, machine-independent generator for 32-bit Microprocessors"
                // https://www.schneier.com/paper-pseudorandom-sequence.html
                uint32_t rnd = random_value;

                rnd = ((((rnd >> 31)
                                ^ (rnd >> 6)
                                ^ (rnd >> 4)
                                ^ (rnd >> 2)
                                ^ (rnd >> 1)
                                ^ rnd)
                            & 0x0000001)
                        << 31 )
                    | (rnd >> 1);

                random_value = rnd;

                result = ((result << 1) | (rnd & 0x00000001));
            } while(m >>= 1);
        } while (result > (uint32_t)max);

        return result;
    }

    /**
     * Seed the random number generator (RNG).
     *
     * default: 0xCAFECAFE
     * @return an unsigned 32 bit seed to seed codal's lightweight Galois LFSR.
     */
    virtual uint32_t seedRandom()
    {
        return 0xCAFECAFE;
    }
};

extern CodalDevice device;

#endif
