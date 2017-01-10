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

#ifndef LIS3DH_H
#define LIS3DH_H

#include "mbed.h"
#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "DevicePin.h"
#include "CoordinateSystem.h"
#include "DeviceI2C.h"

/**
  * Status flags
  */
#define ACCELEROMETER_IMU_DATA_VALID           0x02

/**
  * I2C constants
  */
#define LIS3DH_DEFAULT_ADDR    0x32

/**
  * LIS3DH Register map (partial)
  */
#define LIS3DH_STATUS_REG      0x27
#define LIS3DH_STATUS_REG_AUX  0x07
#define LIS3DH_OUT_X_L         0x28
#define LIS3DH_OUT_X_H         0x29
#define LIS3DH_OUT_Y_L         0x2A
#define LIS3DH_OUT_Y_H         0x2B
#define LIS3DH_OUT_Z_L         0x2C
#define LIS3DH_OUT_Z_H         0x2D
#define LIS3DH_WHOAMI          0x0F
#define LIS3DH_CTRL_REG0       0x1E
#define LIS3DH_CTRL_REG1       0x20
#define LIS3DH_CTRL_REG2       0x21
#define LIS3DH_CTRL_REG3       0x22
#define LIS3DH_CTRL_REG4       0x23
#define LIS3DH_CTRL_REG5       0x24
#define LIS3DH_CTRL_REG6       0x25

#define LIS3DH_FIFO_CTRL_REG   0x2E
#define LIS3DH_FIFO_SRC_REG    0x2F
#define LIS3DH_INT1_CFG        0x30
#define LIS3DH_INT1_SRC        0x31
#define LIS3DH_INT1_THS        0x32
#define LIS3DH_INT1_DURATION   0x33
#define LIS3DH_INT2_CFG        0x34
#define LIS3DH_INT2_SRC        0x35
#define LIS3DH_INT2_THS        0x36
#define LIS3DH_INT2_DURATION   0x37


/**
  * MMA8653 constants
  */
#define LIS3DH_WHOAMI_VAL      0x33

#define LIS3DH_SAMPLE_RANGES   4
#define LIS3DH_SAMPLE_RATES    7

/**
  * Accelerometer events
  */
#define ACCELEROMETER_EVT_DATA_UPDATE              1

/**
  * Gesture events
  */
#define ACCELEROMETER_EVT_NONE                     0
#define ACCELEROMETER_EVT_TILT_UP                  1
#define ACCELEROMETER_EVT_TILT_DOWN                2
#define ACCELEROMETER_EVT_TILT_LEFT                3
#define ACCELEROMETER_EVT_TILT_RIGHT               4
#define ACCELEROMETER_EVT_FACE_UP                  5
#define ACCELEROMETER_EVT_FACE_DOWN                6
#define ACCELEROMETER_EVT_FREEFALL                 7
#define ACCELEROMETER_EVT_3G                       8
#define ACCELEROMETER_EVT_6G                       9
#define ACCELEROMETER_EVT_8G                       10
#define ACCELEROMETER_EVT_SHAKE                    11

/**
  * Gesture recogniser constants
  */
#define ACCELEROMETER_REST_TOLERANCE               200
#define ACCELEROMETER_TILT_TOLERANCE               200
#define ACCELEROMETER_FREEFALL_TOLERANCE           400
#define ACCELEROMETER_SHAKE_TOLERANCE              400
#define ACCELEROMETER_3G_TOLERANCE                 3072
#define ACCELEROMETER_6G_TOLERANCE                 6144
#define ACCELEROMETER_8G_TOLERANCE                 8192
#define ACCELEROMETER_GESTURE_DAMPING              5
#define ACCELEROMETER_SHAKE_DAMPING                10 
#define ACCELEROMETER_SHAKE_RTX                    30

#define ACCELEROMETER_REST_THRESHOLD               (ACCELEROMETER_REST_TOLERANCE * ACCELEROMETER_REST_TOLERANCE)
#define ACCELEROMETER_FREEFALL_THRESHOLD           (ACCELEROMETER_FREEFALL_TOLERANCE * ACCELEROMETER_FREEFALL_TOLERANCE)
#define ACCELEROMETER_3G_THRESHOLD                 (ACCELEROMETER_3G_TOLERANCE * ACCELEROMETER_3G_TOLERANCE)
#define ACCELEROMETER_6G_THRESHOLD                 (ACCELEROMETER_6G_TOLERANCE * ACCELEROMETER_6G_TOLERANCE)
#define ACCELEROMETER_8G_THRESHOLD                 (ACCELEROMETER_8G_TOLERANCE * ACCELEROMETER_8G_TOLERANCE)
#define ACCELEROMETER_SHAKE_COUNT_THRESHOLD        4

struct Sample3D
{
    int16_t         x;
    int16_t         y;
    int16_t         z;
};

struct LIS3DHSampleRateConfig
{
    uint32_t        period;
    uint8_t         value;
};

struct LIS3DHSampleRangeConfig
{
    uint32_t        range;
    uint8_t         value;
};


extern const LIS3DHSampleRangeConfig LIS3DHSampleRange[];
extern const LIS3DHSampleRateConfig LIS3DHSampleRate[];

struct ShakeHistory
{
    uint16_t    shaken:1,
                x:1,
                y:1,
                z:1,
                unused,
                impulse_3,
                impulse_6,
                impulse_8,
                count:8;

    uint16_t    timer;
};

/**
 * Class definition for Accelerometer.
 *
 * Represents an implementation of the Freescale MMA8653 3 axis accelerometer
 * Also includes basic data caching and on demand activation.
 */
class LIS3DH : public DeviceComponent
{
    DeviceI2C&      i2c;                // The I2C interface to use.
    DevicePin       &int1;              // Data ready interrupt.
    uint16_t        address;            // I2C address of this accelerometer.
    uint16_t        samplePeriod;       // The time between samples, in milliseconds.
    uint8_t         sampleRange;        // The sample range of the accelerometer in g.
    Sample3D        sample;             // The last sample read.
    float           pitch;              // Pitch of the device, in radians.
    float           roll;               // Roll of the device, in radians.
    uint8_t         sigma;              // the number of ticks that the instantaneous gesture has been stable.
    uint8_t         impulseSigma;       // the number of ticks since an impulse event has been generated.
    uint16_t        lastGesture;        // the last, stable gesture recorded.
    uint16_t        currentGesture;     // the instantaneous, unfiltered gesture detected.
    ShakeHistory    shake;              // State information needed to detect shake events.

    public:

    /**
      * Constructor.
      * Create a software abstraction of an accelerometer.
      *
      * @param _i2c an instance of DeviceI2C used to communicate with the onboard accelerometer.
      *
      * @param address the default I2C address of the accelerometer. Defaults to: MMA8653_DEFAULT_ADDR.
      *
      * @param id the unique EventModel id of this component. Defaults to: DEVICE_ID_ACCELEROMETER
      *
      * @code
      * DeviceI2C i2c = DeviceI2C(I2C_SDA0, I2C_SCL0);
      *
      * Accelerometer accelerometer = Accelerometer(i2c);
      * @endcode
     */
    LIS3DH(DeviceI2C &_i2c, DevicePin &_int1, uint16_t address = LIS3DH_DEFAULT_ADDR, uint16_t id = DEVICE_ID_ACCELEROMETER);

    /**
      * Configures the accelerometer for G range and sample rate defined
      * in this object. The nearest values are chosen to those defined
      * that are supported by the hardware. The instance variables are then
      * updated to reflect reality.
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the accelerometer could not be configured.
      */
    int configure();

    /**
      * Reads the acceleration data from the accelerometer, and stores it in our buffer.
      * This only happens if the accelerometer indicates that it has new data via int1.
      *
      * On first use, this member function will attempt to add this component to the
      * list of fiber components in order to constantly update the values stored
      * by this object.
      *
      * This technique is called lazy instantiation, and it means that we do not
      * obtain the overhead from non-chalantly adding this component to fiber components.
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the read request fails.
      */
    int updateSample();

    /**
      * Attempts to set the sample rate of the accelerometer to the specified value (in ms).
      *
      * @param period the requested time between samples, in milliseconds.
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR is the request fails.
      *
      * @code
      * // sample rate is now 20 ms.
      * accelerometer.setPeriod(20);
      * @endcode
      *
      * @note The requested rate may not be possible on the hardware. In this case, the
      * nearest lower rate is chosen.
      */
    int setPeriod(int period);

    /**
      * Reads the currently configured sample rate of the accelerometer.
      *
      * @return The time between samples, in milliseconds.
      */
    int getPeriod();

    /**
      * Attempts to set the sample range of the accelerometer to the specified value (in g).
      *
      * @param range The requested sample range of samples, in g.
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR is the request fails.
      *
      * @code
      * // the sample range of the accelerometer is now 8G.
      * accelerometer.setRange(8);
      * @endcode
      *
      * @note The requested range may not be possible on the hardware. In this case, the
      * nearest lower range is chosen.
      */
    int setRange(int range);

    /**
      * Reads the currently configured sample range of the accelerometer.
      *
      * @return The sample range, in g.
      */
    int getRange();

    /**
      * Attempts to read the 8 bit ID from the accelerometer, this can be used for
      * validation purposes.
      *
      * @return the 8 bit ID returned by the accelerometer, or DEVICE_I2C_ERROR if the request fails.
      *
      * @code
      * accelerometer.whoAmI();
      * @endcode
      */
    int whoAmI();

    /**
      * Reads the value of the X axis from the latest update retrieved from the accelerometer.
      *
      * @param system The coordinate system to use. By default, a simple cartesian system is provided.
      *
      * @return The force measured in the X axis, in milli-g.
      *
      * @code
      * accelerometer.getX();
      * @endcode
      */
    int getX(CoordinateSystem system = SIMPLE_CARTESIAN);

    /**
      * Reads the value of the Y axis from the latest update retrieved from the accelerometer.
      *
      * @return The force measured in the Y axis, in milli-g.
      *
      * @code
      * accelerometer.getY();
      * @endcode
      */
    int getY(CoordinateSystem system = SIMPLE_CARTESIAN);

    /**
      * Reads the value of the Z axis from the latest update retrieved from the accelerometer.
      *
      * @return The force measured in the Z axis, in milli-g.
      *
      * @code
      * accelerometer.getZ();
      * @endcode
      */
    int getZ(CoordinateSystem system = SIMPLE_CARTESIAN);

    /**
      * Provides a rotation compensated pitch of the device, based on the latest update retrieved from the accelerometer.
      *
      * @return The pitch of the device, in degrees.
      *
      * @code
      * accelerometer.getPitch();
      * @endcode
      */
    int getPitch();

    /**
      * Provides a rotation compensated pitch of the device, based on the latest update retrieved from the accelerometer.
      *
      * @return The pitch of the device, in radians.
      *
      * @code
      * accelerometer.getPitchRadians();
      * @endcode
      */
    float getPitchRadians();

    /**
      * Provides a rotation compensated roll of the device, based on the latest update retrieved from the accelerometer.
      *
      * @return The roll of the device, in degrees.
      *
      * @code
      * accelerometer.getRoll();
      * @endcode
      */
    int getRoll();

    /**
      * Provides a rotation compensated roll of the device, based on the latest update retrieved from the accelerometer.
      *
      * @return The roll of the device, in radians.
      *
      * @code
      * accelerometer.getRollRadians();
      * @endcode
      */
    float getRollRadians();

    /**
      * Retrieves the last recorded gesture.
      *
      * @return The last gesture that was detected.
      *
      * Example:
      * @code
      *
      * if (accelerometer.getGesture() == SHAKE)
      *     display.scroll("SHAKE!");
      * @endcode
      */
    uint16_t getGesture();

    /**
      * A periodic callback invoked by the fiber scheduler idle thread.
      *
      * Internally calls updateSample().
      */
    virtual void idleCallback();

    /**
      * Destructor.
      */
    ~LIS3DH();

    private:

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
    int writeCommand(uint8_t reg, uint8_t value);

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
    int readCommand(uint8_t reg, uint8_t* buffer, int length);

    /**
      * Recalculate roll and pitch values for the current sample.
      *
      * @note We only do this at most once per sample, as the necessary trigonemteric functions are rather
      *       heavyweight for a CPU without a floating point unit.
      */
    void recalculatePitchRoll();

    /**
      * Updates the basic gesture recognizer. This performs instantaneous pose recognition, and also some low pass filtering to promote
      * stability.
      */
    void updateGesture();

    /**
      * A service function.
      * It calculates the current scalar acceleration of the device (x^2 + y^2 + z^2).
      * It does not, however, square root the result, as this is a relatively high cost operation.
      *
      * This is left to application code should it be needed.
      *
      * @return the sum of the square of the acceleration of the device across all axes.
      */
    int instantaneousAccelerationSquared();

    /**
     * Service function.
     * Determines a 'best guess' posture of the device based on instantaneous data.
     *
     * This makes no use of historic data, and forms this input to the filter implemented in updateGesture().
     *
     * @return A 'best guess' of the current posture of the device, based on instanataneous data.
     */
    uint16_t instantaneousPosture();
};

#endif
