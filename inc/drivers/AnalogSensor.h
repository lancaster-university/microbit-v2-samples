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

#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "DevicePin.h"
#include "DeviceEvent.h"


/**
  * Sensor events
  */
#define ANALOG_THRESHOLD_LOW                           1
#define ANALOG_THRESHOLD_HIGH                          2
#define ANALOG_SENSOR_UPDATE_NEEDED                    3

/**
 * Status values
 */
#define ANALOG_SENSOR_INITIALISED                       0x01
#define ANALOG_SENSOR_HIGH_THRESHOLD_PASSED             0x02
#define ANALOG_SENSOR_LOW_THRESHOLD_PASSED              0x04
#define ANALOG_SENSOR_THRESHOLD_ENABLED                 0x08

/**
 * Class definition for a generic analog sensor, and performs periodic sampling, buffering and low pass filtering of the data.
 */
class AnalogSensor : public DeviceComponent
{
    protected:

    DevicePin       &_pin;              // Pin where the sensor is connected.
    uint16_t        samplePeriod;       // The time between samples, in milliseconds.
    float           sensitivity;        // A value between 0..1 used with a decay average to smooth the sample data. 
    uint16_t        highThreshold;      // threshold at which a HIGH event is generated
    uint16_t        lowThreshold;       // threshold at which a LOW event is generated
    uint16_t        sensorValue;        // Last sampled data.

    public:

    /**
      * Constructor.
      *
      * Creates a generic AnalogSensor. 
      *
      * @param pin The pin on which to sense
      * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER 
     */
    AnalogSensor(DevicePin &pin, uint16_t id);

    /*
     * Event Handler for periodic sample timer
     */
    void onSampleEvent(DeviceEvent);

    /**
     * Updates the internal reading of the sensor. Typically called periodicaly.
     *
     * @return DEVICE_OK on success.
     */
    virtual void updateSample();

    /*
     * Determines the instantaneous value of the sensor, in SI units, and returns it.
     *
     * @return The current value of the sensor.
     */
    float getValue();
    
    /**
      * Set the automatic sample period of the accelerometer to the specified value (in ms).
      *
      * @param period the requested time between samples, in milliseconds.
      *
      * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
      */
    int setPeriod(int period);

    /**
      * Reads the currently configured sample period.
      *
      * @return The time between samples, in milliseconds.
      */
    int getPeriod();

    /**
      * Set HIGH and LOW thresholds to the given value. Events will be generated when these thresholds are crossed.
      *
      * @param low the LOW threshold, in SI units.
      * @param high the HIGH threshold, in SI units.
      *
      * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
      */
    int setThreshold(float low, float high);

    /**
      * Set smoothing value for the data. A decay average is taken of sampled data to smooth it into more accurate information.
      *
      * @param value A value between 0..1 that detemrines the level of smoothing. Set to 0 to disable smoothing. Default value is 0.1
      *
      * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
      */
    int setSensitivity(float value);

    /**
      * Destructor.
      */
    ~AnalogSensor();

    protected:
    /**
     * Determine if any thresholding events need to be generated, and if so, raise them.
     */
    void checkThresholding();


};

#endif
