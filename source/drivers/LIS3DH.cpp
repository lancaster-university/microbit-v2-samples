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

/**
 * Class definition for an LIS3DH 3 axis accelerometer.
 *
 * Represents an implementation of the Freescale LIS3DH 3 axis accelerometer
 * Also includes basic data caching and on demand activation.
 */
#include "DeviceConfig.h"
#include "LIS3DH.h"
#include "ErrorNo.h"
#include "DeviceConfig.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceFiber.h"

/**
  * Configures the accelerometer for G range and sample rate defined
  * in this object. The nearest values are chosen to those defined
  * that are supported by the hardware. The instance variables are then
  * updated to reflect reality.
  *
  * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the accelerometer could not be configured.
  */
int LIS3DH::configure()
{
    const LIS3DHSampleRangeConfig  *actualSampleRange;
    const LIS3DHSampleRateConfig  *actualSampleRate;
    int result;

    // First find the nearest sample rate to that specified.
    actualSampleRate = &LIS3DHSampleRate[LIS3DH_SAMPLE_RATES-1];
    for (int i=LIS3DH_SAMPLE_RATES-1; i>=0; i--)
    {
        if(LIS3DHSampleRate[i].period < this->samplePeriod * 1000)
            break;

        actualSampleRate = &LIS3DHSampleRate[i];
    }

    // Now find the nearest sample range to that specified.
    actualSampleRange = &LIS3DHSampleRange[LIS3DH_SAMPLE_RANGES-1];
    for (int i=LIS3DH_SAMPLE_RANGES-1; i>=0; i--)
    {
        if(LIS3DHSampleRange[i].range < this->sampleRange)
            break;

        actualSampleRange = &LIS3DHSampleRange[i];
    }

    // OK, we have the correct data. Update our local state.
    this->samplePeriod = actualSampleRate->period / 1000;
    this->sampleRange = actualSampleRange->range;

    // Now configure the accelerometer accordingly.

    // Firstly, Configure for normal precision operation at the sample rate requested.
    result = writeCommand(LIS3DH_CTRL_REG1, actualSampleRate->value | 0x07);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    // Enable the INT1 interrupt pin when XYZ data is available.
    result = writeCommand(LIS3DH_CTRL_REG3, 0x10);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    // Configure for the selected g range.
    result = writeCommand(LIS3DH_CTRL_REG4,  actualSampleRange->value << 4);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    // Configure for a latched interrupt request.
    result = writeCommand(LIS3DH_CTRL_REG5, 0x08);
    if (result != 0)
        return DEVICE_I2C_ERROR;

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
int LIS3DH::writeCommand(uint8_t reg, uint8_t value)
{
    uint8_t command[2];
    command[0] = reg;
    command[1] = value;

    return i2c.write(address, (const char *)command, 2);
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
int LIS3DH::readCommand(uint8_t reg, uint8_t* buffer, int length)
{
    int result;

    if (buffer == NULL || length <= 0 )
        return DEVICE_INVALID_PARAMETER;

    result = i2c.write(address, (const char *)&reg, 1, true);
    if (result !=0)
        return DEVICE_I2C_ERROR;

    result = i2c.read(address, (char *)buffer, length);
    if (result !=0)
        return DEVICE_I2C_ERROR;

    return DEVICE_OK;
}

/**
  * Constructor.
  * Create a software abstraction of an accelerometer.
  *
  * @param _i2c an instance of DeviceI2C used to communicate with the onboard accelerometer.
  *
  * @param address the default I2C address of the accelerometer. Defaults to: LIS3DH_DEFAULT_ADDR.
  *
  * @param id the unique EventModel id of this component. Defaults to: DEVICE_ID_ACCELEROMETER
  *
  * @code
  * DeviceI2C i2c = DeviceI2C(I2C_SDA0, I2C_SCL0);
  *
  * LIS3DH accelerometer = LIS3DH(i2c);
  * @endcode
 */
LIS3DH::LIS3DH(DeviceI2C& _i2c, DevicePin &_int1, uint16_t address,  uint16_t id) : i2c(_i2c), int1(_int1), sample()
{
    // Store our identifiers.
    this->id = id;
    this->status = 0;
    this->address = address;

    // Update our internal state for 50Hz at +/- 2g (50Hz has a period af 20ms).
    this->samplePeriod = 20;
    this->sampleRange = 2;

    // Initialise gesture history
    this->sigma = 0;
    this->impulseSigma = 0;
    this->lastGesture = ACCELEROMETER_EVT_NONE;
    this->currentGesture = ACCELEROMETER_EVT_NONE;
    this->shake.x = 0;
    this->shake.y = 0;
    this->shake.z = 0;
    this->shake.count = 0;
    this->shake.timer = 0;
    this->shake.impulse_3 = 1;
    this->shake.impulse_6 = 1;
    this->shake.impulse_8 = 1;

    // Configure and enable the accelerometer.
    configure();
}

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
int LIS3DH::whoAmI()
{
    uint8_t data;
    int result;

    result = readCommand(LIS3DH_WHOAMI, &data, 1);
    if (result !=0)
        return DEVICE_I2C_ERROR;

    return (int)data;
}

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
int LIS3DH::updateSample()
{
    // Ensure we're scheduled to update the data periodically
    status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;

    // Poll interrupt line from accelerometer.
    if(int1.getDigitalValue() == 1)
    {
        int8_t data[6];
        uint8_t src;
        int result;

        // read the XYZ data (16 bit)
        // n.b. we need to set the MSB bit to enable multibyte transfers from this device (WHY? Who Knows!)
        result = readCommand(0x80 | LIS3DH_OUT_X_L, (uint8_t *)data, 6);

        if (result !=0)
            return DEVICE_I2C_ERROR;

        // Acknowledge the interrupt.
        readCommand(LIS3DH_INT1_SRC, &src, 1);

        // read MSB values...
        sample.x = data[1];
        sample.y = data[3];
        sample.z = data[5];

        // Normalize the data in the 0..1024 range.
        sample.x *= 8;
        sample.y *= 8;
        sample.z *= 8;

#if CONFIG_ENABLED(USE_ACCEL_LSB)
        // Add in LSB values.
        sample.x += (data[0] / 64);
        sample.y += (data[2] / 64);
        sample.z += (data[4] / 64);
#endif

        // Scale into millig (approx!)
        sample.x *= this->sampleRange;
        sample.y *= this->sampleRange;
        sample.z *= this->sampleRange;

        // Indicate that pitch and roll data is now stale, and needs to be recalculated if needed.
        status &= ~ACCELEROMETER_IMU_DATA_VALID;

        // Update gesture tracking
        updateGesture();

        // Indicate that a new sample is available
        DeviceEvent e(id, ACCELEROMETER_EVT_DATA_UPDATE);
    }

    return DEVICE_OK;
};

/**
  * A service function.
  * It calculates the current scalar acceleration of the device (x^2 + y^2 + z^2).
  * It does not, however, square root the result, as this is a relatively high cost operation.
  *
  * This is left to application code should it be needed.
  *
  * @return the sum of the square of the acceleration of the device across all axes.
  */
int LIS3DH::instantaneousAccelerationSquared()
{
    updateSample();

    // Use pythagoras theorem to determine the combined force acting on the device.
    return (int)sample.x*(int)sample.x + (int)sample.y*(int)sample.y + (int)sample.z*(int)sample.z;
}

/**
 * Service function.
 * Determines a 'best guess' posture of the device based on instantaneous data.
 *
 * This makes no use of historic data, and forms the input to the filter implemented in updateGesture().
 *
 * @return A 'best guess' of the current posture of the device, based on instanataneous data.
 */
uint16_t LIS3DH::instantaneousPosture()
{
    bool shakeDetected = false;

    // Test for shake events.
    // We detect a shake by measuring zero crossings in each axis. In other words, if we see a strong acceleration to the left followed by
    // a strong acceleration to the right, then we can infer a shake. Similarly, we can do this for each axis (left/right, up/down, in/out).
    //
    // If we see enough zero crossings in succession (ACCELEROMETER_SHAKE_COUNT_THRESHOLD), then we decide that the device
    // has been shaken.
    if ((getX() < -ACCELEROMETER_SHAKE_TOLERANCE && shake.x) || (getX() > ACCELEROMETER_SHAKE_TOLERANCE && !shake.x))
    {
        shakeDetected = true;
        shake.x = !shake.x;
    }

    if ((getY() < -ACCELEROMETER_SHAKE_TOLERANCE && shake.y) || (getY() > ACCELEROMETER_SHAKE_TOLERANCE && !shake.y))
    {
        shakeDetected = true;
        shake.y = !shake.y;
    }

    if ((getZ() < -ACCELEROMETER_SHAKE_TOLERANCE && shake.z) || (getZ() > ACCELEROMETER_SHAKE_TOLERANCE && !shake.z))
    {
        shakeDetected = true;
        shake.z = !shake.z;
    }

    // If we detected a zero crossing in this sample period, count this.
    if (shakeDetected && shake.count < ACCELEROMETER_SHAKE_COUNT_THRESHOLD)
    {
        shake.count++;
  
        if (shake.count == 1)
            shake.timer = 0;

        if (shake.count == ACCELEROMETER_SHAKE_COUNT_THRESHOLD)
        {
            shake.shaken = 1;
            shake.timer = 0;
            return ACCELEROMETER_EVT_SHAKE;
        }
    }

    // measure how long we have been detecting a SHAKE event.
    if (shake.count > 0)
    {
        shake.timer++;

        // If we've issued a SHAKE event already, and sufficient time has assed, allow another SHAKE event to be issued.
        if (shake.shaken && shake.timer >= ACCELEROMETER_SHAKE_RTX)
        {
            shake.shaken = 0;
            shake.timer = 0;
            shake.count = 0;
        }

        // Decay our count of zero crossings over time. We don't want them to accumulate if the user performs slow moving motions.
        else if (!shake.shaken && shake.timer >= ACCELEROMETER_SHAKE_DAMPING)
        {
            shake.timer = 0;
            if (shake.count > 0)
                shake.count--;
        }
    }

    if (instantaneousAccelerationSquared() < ACCELEROMETER_FREEFALL_THRESHOLD)
        return ACCELEROMETER_EVT_FREEFALL;

    // Determine our posture.
    if (getX() < (-1000 + ACCELEROMETER_TILT_TOLERANCE))
        return ACCELEROMETER_EVT_TILT_LEFT;

    if (getX() > (1000 - ACCELEROMETER_TILT_TOLERANCE))
        return ACCELEROMETER_EVT_TILT_RIGHT;

    if (getY() < (-1000 + ACCELEROMETER_TILT_TOLERANCE))
        return ACCELEROMETER_EVT_TILT_DOWN;

    if (getY() > (1000 - ACCELEROMETER_TILT_TOLERANCE))
        return ACCELEROMETER_EVT_TILT_UP;

    if (getZ() < (-1000 + ACCELEROMETER_TILT_TOLERANCE))
        return ACCELEROMETER_EVT_FACE_UP;

    if (getZ() > (1000 - ACCELEROMETER_TILT_TOLERANCE))
        return ACCELEROMETER_EVT_FACE_DOWN;

    return ACCELEROMETER_EVT_NONE;
}

/**
  * Updates the basic gesture recognizer. This performs instantaneous pose recognition, and also some low pass filtering to promote
  * stability.
  */
void LIS3DH::updateGesture()
{
    // Check for High/Low G force events - typically impulses, impacts etc.
    // Again, during such spikes, these event take priority of the posture of the device.
    // For these events, we don't perform any low pass filtering.
    int force = instantaneousAccelerationSquared();

    if (force > ACCELEROMETER_3G_THRESHOLD)
    {
        if (force > ACCELEROMETER_3G_THRESHOLD && !shake.impulse_3)
        {
            DeviceEvent e(DEVICE_ID_GESTURE, ACCELEROMETER_EVT_3G);
            shake.impulse_3 = 1;
        }
        if (force > ACCELEROMETER_6G_THRESHOLD && !shake.impulse_6)
        {
            DeviceEvent e(DEVICE_ID_GESTURE, ACCELEROMETER_EVT_6G);
            shake.impulse_6 = 1;
        }
        if (force > ACCELEROMETER_8G_THRESHOLD && !shake.impulse_8)
        {
            DeviceEvent e(DEVICE_ID_GESTURE, ACCELEROMETER_EVT_8G);
            shake.impulse_8 = 1;
        }

        impulseSigma = 0;
    }

    // Reset the impulse event onve the acceleration has subsided.
    if (impulseSigma < ACCELEROMETER_GESTURE_DAMPING)
        impulseSigma++;
    else
        shake.impulse_3 = shake.impulse_6 = shake.impulse_8 = 0;


    // Determine what it looks like we're doing based on the latest sample...
    uint16_t g = instantaneousPosture();

    if (g == ACCELEROMETER_EVT_SHAKE)
    {
        DeviceEvent e(DEVICE_ID_GESTURE, ACCELEROMETER_EVT_SHAKE);
        return;
    }

    // Perform some low pass filtering to reduce jitter from any detected effects
    if (g == currentGesture)
    {
        if (sigma < ACCELEROMETER_GESTURE_DAMPING)
            sigma++;
    }
    else
    {
        currentGesture = g;
        sigma = 0;
    }

    // If we've reached threshold, update our record and raise the relevant event...
    if (currentGesture != lastGesture && sigma >= ACCELEROMETER_GESTURE_DAMPING)
    {
        lastGesture = currentGesture;
        DeviceEvent e(DEVICE_ID_GESTURE, lastGesture);
    }
}

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
int LIS3DH::setPeriod(int period)
{
    this->samplePeriod = period;
    return this->configure();
}

/**
  * Reads the currently configured sample rate of the accelerometer.
  *
  * @return The time between samples, in milliseconds.
  */
int LIS3DH::getPeriod()
{
    return (int)samplePeriod;
}

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
int LIS3DH::setRange(int range)
{
    this->sampleRange = range;
    return this->configure();
}

/**
  * Reads the currently configured sample range of the accelerometer.
  *
  * @return The sample range, in g.
  */
int LIS3DH::getRange()
{
    return (int)sampleRange;
}

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
int LIS3DH::getX(CoordinateSystem system)
{
    updateSample();

    switch (system)
    {
        case SIMPLE_CARTESIAN:
            return -sample.x;

        case NORTH_EAST_DOWN:
            return sample.y;

        case RAW:
        default:
            return sample.x;
    }
}

/**
  * Reads the value of the Y axis from the latest update retrieved from the accelerometer.
  *
  * @return The force measured in the Y axis, in milli-g.
  *
  * @code
  * accelerometer.getY();
  * @endcode
  */
int LIS3DH::getY(CoordinateSystem system)
{
    updateSample();

    switch (system)
    {
        case SIMPLE_CARTESIAN:
            return -sample.y;

        case NORTH_EAST_DOWN:
            return -sample.x;

        case RAW:
        default:
            return sample.y;
    }
}

/**
  * Reads the value of the Z axis from the latest update retrieved from the accelerometer.
  *
  * @return The force measured in the Z axis, in milli-g.
  *
  * @code
  * accelerometer.getZ();
  * @endcode
  */
int LIS3DH::getZ(CoordinateSystem system)
{
    updateSample();

    switch (system)
    {
        case NORTH_EAST_DOWN:
            return -sample.z;

        case SIMPLE_CARTESIAN:
        case RAW:
        default:
            return sample.z;
    }
}

/**
  * Provides a rotation compensated pitch of the device, based on the latest update retrieved from the accelerometer.
  *
  * @return The pitch of the device, in degrees.
  *
  * @code
  * accelerometer.getPitch();
  * @endcode
  */
int LIS3DH::getPitch()
{
    return (int) ((360*getPitchRadians()) / (2*PI));
}

/**
  * Provides a rotation compensated pitch of the device, based on the latest update retrieved from the accelerometer.
  *
  * @return The pitch of the device, in radians.
  *
  * @code
  * accelerometer.getPitchRadians();
  * @endcode
  */
float LIS3DH::getPitchRadians()
{
    if (!(status & ACCELEROMETER_IMU_DATA_VALID))
        recalculatePitchRoll();

    return pitch;
}

/**
  * Provides a rotation compensated roll of the device, based on the latest update retrieved from the accelerometer.
  *
  * @return The roll of the device, in degrees.
  *
  * @code
  * accelerometer.getRoll();
  * @endcode
  */
int LIS3DH::getRoll()
{
    return (int) ((360*getRollRadians()) / (2*PI));
}

/**
  * Provides a rotation compensated roll of the device, based on the latest update retrieved from the accelerometer.
  *
  * @return The roll of the device, in radians.
  *
  * @code
  * accelerometer.getRollRadians();
  * @endcode
  */
float LIS3DH::getRollRadians()
{
    if (!(status & ACCELEROMETER_IMU_DATA_VALID))
        recalculatePitchRoll();

    return roll;
}

/**
  * Recalculate roll and pitch values for the current sample.
  *
  * @note We only do this at most once per sample, as the necessary trigonemteric functions are rather
  *       heavyweight for a CPU without a floating point unit.
  */
void LIS3DH::recalculatePitchRoll()
{
    double x = (double) getX(NORTH_EAST_DOWN);
    double y = (double) getY(NORTH_EAST_DOWN);
    double z = (double) getZ(NORTH_EAST_DOWN);

    roll = atan2(y, z);
    pitch = atan(-x / (y*sin(roll) + z*cos(roll)));

    status |= ACCELEROMETER_IMU_DATA_VALID;
}

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
uint16_t LIS3DH::getGesture()
{
    return lastGesture;
}

/**
  * A periodic callback invoked by the fiber scheduler idle thread.
  *
  * Internally calls updateSample().
  */
void LIS3DH::idleCallback()
{
    updateSample();
}

/**
  * Destructor for LIS3DH, where we deregister from the array of fiber components.
  */
LIS3DH::~LIS3DH()
{
}

const LIS3DHSampleRangeConfig LIS3DHSampleRange[LIS3DH_SAMPLE_RANGES] = {
    {2, 0},
    {4, 1},
    {8, 2},
    {16, 3}
};

const LIS3DHSampleRateConfig LIS3DHSampleRate[LIS3DH_SAMPLE_RATES] = {
    {2500,      0x70},
    {5000,      0x60},
    {10000,     0x50},
    {20000,     0x40},
    {40000,     0x30},
    {100000,    0x20},
    {1000000,   0x10}
};
