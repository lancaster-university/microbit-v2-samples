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
 * Class definition for a normalised, non-linear analog sensor, that takes the general form of a logarithmic response to a sensed value, in a potential divider.
 * Implements a base class for such a sensor, using the Steinhart-Hart equation to delineate a result.
 */

#include "DeviceConfig.h"
#include "LinearAnalogSensor.h"
#include "ErrorNo.h"
#include "DeviceConfig.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceFiber.h"
#include "Timer.h"

using namespace codal;

/**
 * Constructor.
 *
 * Creates a LinearAnalogSensor.
 *
 * @param pin The pin on which to sense
 * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER
 * @param inputFloor The minimum level in the input range.
 * @param inputCeiling The maximum level in the input range.
 * @param outputFloor The minimum level in the output range. Default: 0.
 * @param outputCeiling The maximum level in the output range. Default: 1023.
 *
 */
LinearAnalogSensor::LinearAnalogSensor(Pin &pin, uint16_t id, uint16_t inputFloor, uint16_t inputCeiling, float outputFloor, float outputCeiling) : AnalogSensor(pin, id)
{
    this->inputFloor = inputFloor;
    this->outputFloor = outputFloor;
    this->conversionFactor = (outputCeiling - outputFloor) / ((float)(inputCeiling - inputFloor));
}

/**
 * Updates the internal reading of the sensor. Typically called periodicaly.
 *
 * @return DEVICE_OK on success.
 */
void LinearAnalogSensor::updateSample()
{
    float sensorReading, value;

    sensorReading = (float) (_pin.getAnalogValue() - inputFloor);

    value = outputFloor + sensorReading * conversionFactor;

    // If this is the first reading performed, take it a a baseline. Otherwise, perform a decay average to smooth out the data.
    if (!(status & ANALOG_SENSOR_INITIALISED))
    {
        sensorValue = value;
        status |=  ANALOG_SENSOR_INITIALISED;
    }
    else
    {
        sensorValue = (sensorValue * (1.0f - sensitivity)) + (value * sensitivity);
    }

    checkThresholding();
}
