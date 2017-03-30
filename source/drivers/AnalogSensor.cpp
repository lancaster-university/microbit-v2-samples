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
#include "Timer.h"

using namespace codal;

/**
 * Constructor.
 *
 * Creates a generic AnalogSensor.
 *
 * @param pin The pin on which to sense
 * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER
 */
AnalogSensor::AnalogSensor(Pin &pin, uint16_t id) : _pin(pin)
{
    this->id = id;
    this->sensitivity = 868;

    // Configure for a 2 Hz update frequency by default.
    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->listen(id, ANALOG_SENSOR_UPDATE_NEEDED, this, &AnalogSensor::onSampleEvent, MESSAGE_BUS_LISTENER_IMMEDIATE);

    setPeriod(500);
    updateSample();
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
int AnalogSensor::getValue()
{
    return sensorValue;
}

/**
 * Updates the internal reading of the sensor. Typically called periodicaly.
 */
void AnalogSensor::updateSample()
{
    int value = _pin.getAnalogValue();

    // If this is the first reading performed, take it a a baseline. Otherwise, perform a decay average to smooth out the data.
    if (!(status & ANALOG_SENSOR_INITIALISED))
    {
        sensorValue = value;
        status |=  ANALOG_SENSOR_INITIALISED;
    }
    else
    {
        sensorValue = ((sensorValue * (1023 - sensitivity)) + (value * sensitivity)) >> 10;
    }

    checkThresholding();
}

/**
 * Determine if any thresholding events need to be generated, and if so, raise them.
 */
void AnalogSensor::checkThresholding()
{
    if ((status & ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED) && (!(status & ANALOG_SENSOR_HIGH_THRESHOLD_PASSED)) && (sensorValue >= highThreshold))
    {
        DeviceEvent(id, ANALOG_THRESHOLD_HIGH);
        status |=  ANALOG_SENSOR_HIGH_THRESHOLD_PASSED;
        status &= ~ANALOG_SENSOR_LOW_THRESHOLD_PASSED;
    }

    if ((status & ANALOG_SENSOR_LOW_THRESHOLD_ENABLED) && (!(status & ANALOG_SENSOR_LOW_THRESHOLD_PASSED)) && (sensorValue <= lowThreshold))

    {
        DeviceEvent(id, ANALOG_THRESHOLD_LOW);
        status |=  ANALOG_SENSOR_LOW_THRESHOLD_PASSED;
        status &= ~ANALOG_SENSOR_HIGH_THRESHOLD_PASSED;
    }
}

/**
 * Set sensitivity value for the data. A decay average is taken of sampled data to smooth it into more accurate information.
 *
 * @param value A value between 0..1023 that detemrines the level of smoothing. Set to 1023 to disable smoothing. Default value is 868
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogSensor::setSensitivity(uint16_t value)
{
    this->sensitivity = max(0, min(1023, value));

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
 * Set threshold to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param value the LOW threshold at which a ANALOG_THRESHOLD_LOW will be generated.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogSensor::setLowThreshold(uint16_t value)
{
    // Protect against churn if the same threshold is set repeatedly.
    if ((status & ANALOG_SENSOR_LOW_THRESHOLD_ENABLED) && lowThreshold == value)
        return DEVICE_OK;

    // We need to update our threshold
    lowThreshold = value;

    // Reset any exisiting threshold state, and enable threshold detection.
    status &= ~ANALOG_SENSOR_LOW_THRESHOLD_PASSED;
    status |=  ANALOG_SENSOR_LOW_THRESHOLD_ENABLED;

    // If a HIGH threshold has been set, ensure it's above the LOW threshold.
    if(status & ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED)
        setHighThreshold(max(lowThreshold+1, highThreshold));

    return DEVICE_OK;
}

/**
 * Set threshold to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param value the HIGH threshold at which a ANALOG_THRESHOLD_HIGH will be generated.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int AnalogSensor::setHighThreshold(uint16_t value)
{
    // Protect against churn if the same threshold is set repeatedly.
    if ((status & ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED) && highThreshold == value)
        return DEVICE_OK;

    // We need to update our threshold
    highThreshold = value;

    // Reset any exisiting threshold state, and enable threshold detection.
    status &= ~ANALOG_SENSOR_HIGH_THRESHOLD_PASSED;
    status |=  ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED;

    // If a HIGH threshold has been set, ensure it's above the LOW threshold.
    if(status & ANALOG_SENSOR_LOW_THRESHOLD_ENABLED)
        setLowThreshold(min(highThreshold - 1, lowThreshold));

    return DEVICE_OK;
}

/**
 * Determines the currently defined low threshold.
 *
 * @return The current low threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
 */
int AnalogSensor::getLowThreshold()
{
    if (!(status & ANALOG_SENSOR_LOW_THRESHOLD_ENABLED))
        return DEVICE_INVALID_PARAMETER;

    return lowThreshold;
}

/**
 * Determines the currently defined high threshold.
 *
 * @return The current high threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
 */
int AnalogSensor::getHighThreshold()
{
    if (!(status & ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED))
        return DEVICE_INVALID_PARAMETER;

    return highThreshold;
}

/**
 * Destructor.
 */
AnalogSensor::~AnalogSensor()
{
}
