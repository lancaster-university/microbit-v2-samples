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
#include "Pin.h"
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
#define ANALOG_SENSOR_LOW_THRESHOLD_ENABLED             0x08
#define ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED            0x10


namespace codal
{
    /**
     * Class definition for a generic analog sensor, and performs periodic sampling, buffering and low pass filtering of the data.
     */
    class AnalogSensor : public DeviceComponent
    {
        protected:

        Pin             &_pin;              // Pin where the sensor is connected.
        uint16_t        samplePeriod;       // The time between samples, in milliseconds.
        uint16_t        sensitivity;        // A value between 0..1023 used with a decay average to smooth the sample data.
        uint16_t        highThreshold;      // threshold at which a HIGH event is generated
        uint16_t        lowThreshold;       // threshold at which a LOW event is generated
        int             sensorValue;        // Last sampled data.

        public:

        /**
          * Constructor.
          *
          * Creates a generic AnalogSensor.
          *
          * @param pin The pin on which to sense
          * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER
         */
        AnalogSensor(Pin &pin, uint16_t id);

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
        int getValue();

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
          * Set threshold to the given value. Events will be generated when these thresholds are crossed.
          *
          * @param value the LOW threshold at which a ANALOG_THRESHOLD_LOW will be generated.
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
          */
        int setLowThreshold(uint16_t value);

        /**
          * Set threshold to the given value. Events will be generated when these thresholds are crossed.
          *
          * @param value the HIGH threshold at which a ANALOG_THRESHOLD_HIGH will be generated.
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
          */
        int setHighThreshold(uint16_t value);

        /**
          * Determines the currently defined low threshold.
          *
          * @return The current low threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
          */
        int getLowThreshold();

        /**
          * Determines the currently defined high threshold.
          *
          * @return The current high threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
          */
        int getHighThreshold();

        /**
          * Set smoothing value for the data. A decay average is taken of sampled data to smooth it into more accurate information.
          *
          * @param value A value between 0..1023 that detemrines the level of smoothing. Set to 1023 to disable smoothing. Default value is 868
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
          */
        int setSensitivity(uint16_t value);

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
}

#endif
