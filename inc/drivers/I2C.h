#ifndef CODAL_I2C_H
#define CODAL_I2C_H

#include "device_types.h"
#include "ErrorNo.h"

namespace codal
{
    class I2C
    {

        public:
        /** Create an I2C interface, connected to the specified pins
          *
          *  @param sda I2C data line pin
          *  @param scl I2C clock line pin
          */
        I2C(PinName sda, PinName scl)
        {
        }

        /** Set the frequency of the I2C interface
          *
          * @param frequency The bus frequency in hertz
          *
          * @return DEVICE_OK on success
          */
        virtual int setFrequency(uint32_t frequency)
        {
            return DEVICE_NOT_IMPLEMENTED;
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
        virtual int write(uint32_t address, uint8_t reg, uint8_t value)
        {
            return DEVICE_NOT_IMPLEMENTED;
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
        virtual int read(uint32_t address, uint8_t reg, uint8_t* buffer, int length)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }
    };
}

#endif
