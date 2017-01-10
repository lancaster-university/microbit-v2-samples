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
 * Class definition for a generic analog sensor, that takes the general form of a logarithmic response to a sensed value, in a potential divider.
 * Implements a base class for such a sensor, using the Steinhart-Hart equation to delineate a result.
 */

#include "DeviceConfig.h"
#include "AnalogSensor.h"
#include "ErrorNo.h"
#include "DeviceConfig.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceFiber.h"
#include "DeviceSystemTimer.h"

/**
 * Constructor.
 *
 * Creates a generic AnalogSensor. 
 *
 * @param pin The pin on which to sense
 * @param nominalValue The value (in SI units) of a nominal position.
 * @param nominalReading The raw reading from the sensor at the nominal position.
 * @param beta The Steinhart-Hart Beta constant for the device
 * @param seriesResistor The value (in ohms) of the resistor in series with the sensor.
 * @param zeroOffset Optional zero offset applied to all SI units (e.g. 273.15 for temperature sensing in C vs Kelvin). 
 */
AnalogSensor::AnalogSensor(DevicePin &pin, uint16_t id, float nominalValue, float nominalReading, float beta, float seriesResistor, float zeroOffset) : _pin(pin)
{
    this->id = id;
    this->sensitivity = 0.1;
    this->nominalValue = nominalValue;
    this->nominalReading = nominalReading;
    this->beta = beta;
    this->seriesResistor = seriesResistor;
    this->zeroOffset = zeroOffset;

    // Configure for a 2 Hz update frequency by default. 
    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->listen(id, ANALOG_SENSOR_UPDATE_NEEDED, this, &AnalogSensor::onSampleEvent);

    setPeriod(500);

}

/*
 * Event Handler for periodic sample timer
 */
void AnalogSensor::onSampleEvent(DeviceEvent)
{
    updateSample();
}

/*
 * Determines the instantaneous value of the sensor, in SI units, and returns it.
 *
 * @return The current value of the sensor.
 */
float AnalogSensor::getValue()
{
    return sensorValue;
}

/**
 * Updates the internal reading of the sensor. Typically called periodicaly.
 *
 * @return DEVICE_OK on success.
 */
int AnalogSensor::updateSample()
{
    float sensorReading, value;

    sensorReading = (((1023.0) * seriesResistor) / _pin.getAnalogValue()) - seriesResistor;
    value = (1.0 / ((log(sensorReading / nominalReading) / beta) + (1.0 / (nominalValue + zeroOffset)))) - zeroOffset;

    // If this is the first reading performed, take it a a baseline. Otherwise, perform a decay average to smooth out the data.
    if (!(status & ANALOG_SENSOR_INITIALISED))
    {
        sensorValue = value;
        status |=  ANALOG_SENSOR_INITIALISED;
    }
    else
    {
        sensorValue = (sensorValue * (1-sensitivity)) + (value * sensitivity);
    }

    if (status & ANALOG_SENSOR_THRESHOLD_ENABLED)
    {
        if (((status & ANALOG_SENSOR_HIGH_THRESHOLD_PASSED)==0) && (sensorValue >= highThreshold))
        {
            DeviceEvent(id, ANALOG_THRESHOLD_HIGH);
            status |=  ANALOG_SENSOR_HIGH_THRESHOLD_PASSED;
            status &= ~ANALOG_SENSOR_LOW_THRESHOLD_PASSED;
        }

        if (((status & ANALOG_SENSOR_LOW_THRESHOLD_PASSED)==0) && (sensorValue <= lowThreshold))
        {
            DeviceEvent(id, ANALOG_THRESHOLD_LOW);
            status |=  ANALOG_SENSOR_LOW_THRESHOLD_PASSED;
            status &= ~ANALOG_SENSOR_HIGH_THRESHOLD_PASSED;
        }
    }

    return DEVICE_OK;
}

/**
 * Set sensitivity value for the data. A decay average is taken of sampled data to smooth it into more accurate information.
 *
 * @param value A value between 0..1 that detemrines the level of smoothing. Set to 0 to disable smoothing. Default value is 0.1
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogSensor::setSensitivity(float value)
{
    this->sensitivity = value;

    return DEVICE_OK;
}

/**
 * Set the automatic sample period of the accelerometer to the specified value (in ms).
 *
 * @param period the requested time between samples, in milliseconds.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogSensor::setPeriod(int period)
{
    samplePeriod = period;
    system_timer_event_every_us(1000 * period, id, ANALOG_SENSOR_UPDATE_NEEDED);

    return DEVICE_OK;
}

/**
 * Reads the currently configured sample period.
 *
 * @return The time between samples, in milliseconds.
 */
int AnalogSensor::getPeriod()
{
    return samplePeriod;
}

/**
 * Set HIGH and LOW thresholds to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param low the LOW threshold, in SI units.
 * @param high the HIGH threshold, in SI units.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogSensor::setThreshold(float low, float high)
{
    lowThreshold = low;
    highThreshold = high;

    // Reset any exisiting threshold state, and enable threshold detection.
    status &= ~ANALOG_SENSOR_HIGH_THRESHOLD_PASSED;
    status &= ~ANALOG_SENSOR_LOW_THRESHOLD_PASSED;
    status |=  ANALOG_SENSOR_THRESHOLD_ENABLED;

    return DEVICE_OK;
}

/**
 * Destructor.
 */
AnalogSensor::~AnalogSensor()
{
}


