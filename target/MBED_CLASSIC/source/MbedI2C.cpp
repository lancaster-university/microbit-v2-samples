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

#include "DeviceConfig.h"
#include "MbedI2C.h"
#include "ErrorNo.h"

namespace codal
{
    namespace mbed
    {
        /**
          * Constructor.
          */
        I2C::I2C(PinName sda, PinName scl) : codal::I2C(sda, scl), mb::I2C(sda,scl)
        {

        }

        /** Set the frequency of the I2C interface
          *
          * @param frequency The bus frequency in hertz
          */
        int I2C::setFrequency(uint32_t frequency)
        {
            mb::I2C::frequency(frequency);
            return DEVICE_OK;
        }

        /**
          * Issues a standard, 2 byte I2C command write to the accelerometer.
          *
          * Blocks the calling thread until complete.
          *
          * @param reg The address of the register to write to.
          *
          * @param value The value to write.
          *
          * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
          */
        int I2C::write(uint32_t address, uint8_t reg, uint8_t value)
        {
            uint8_t command[2];
            command[0] = reg;
            command[1] = value;

            return mb::I2C::write(address, (const char *)command, 2);
        }

        /**
          * Issues a read command, copying data into the specified buffer.
          *
          * Blocks the calling thread until complete.
          *
          * @param reg The address of the register to access.
          *
          * @param buffer Memory area to read the data into.
          *
          * @param length The number of bytes to read.
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER or DEVICE_I2C_ERROR if the the read request failed.
          */
        int I2C::read(uint32_t address, uint8_t reg, uint8_t* buffer, int length)
        {
            int result;

            if (buffer == NULL || length <= 0 )
                return DEVICE_INVALID_PARAMETER;

            result = mb::I2C::write(address, (const char *)&reg, 1, true);
            if (result !=0)
                return DEVICE_I2C_ERROR;

            result = mb::I2C::read(address, (char *)buffer, length);
            if (result !=0)
                return DEVICE_I2C_ERROR;

            return DEVICE_OK;
        }
    }
}
