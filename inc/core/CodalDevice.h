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
#include "device_types.h"
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
        if (random_value == 0)
            random_value = 0xC0DA1;
    }

    /**
      * Perform a hard reset of the device.
      * default: Use CMSIS NVIC reset vector.
      */
    virtual void reset() { }

    /**
      * Disable global interrupts.
      */
    virtual void disableInterrupts() { }

    /**
      * Enable global interrupts.
      */
    virtual void enableInterrupts() { }

    /**
      * Enable global interrupts.
      */
    virtual void waitForEvent() { }

    /**
      * returns the current stack pointer
      */
    virtual PROCESSOR_WORD_TYPE getMSP() { return 0; }

    /**
     * Determine the version of system software currently running.
     * default: The version of the codal runtime being used.
     * @return a pointer to a NULL terminated character buffer containing a representation of the current version using semantic versioning.
     */
    virtual const char *
    getVersion()
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
    virtual void panic(int statusCode);

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
    int random(int max);

    /**
     * Seed the random number generator (RNG).
     *
     * @param seed an unsigned 32 bit value used to seed codal's lightweight Galois LFSR.
     * @return DEVICE_OK on success
     */
    virtual int seedRandom(uint32_t seed)
    {
        random_value = seed;
        return DEVICE_OK;
    }
};

extern CodalDevice& device;

#endif
