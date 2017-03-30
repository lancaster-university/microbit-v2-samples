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

#ifndef NONLINEAR_ANALOG_SENSOR_H
#define NONLINEAR_ANALOG_SENSOR_H

#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "Pin.h"
#include "DeviceEvent.h"
#include "AnalogSensor.h"

namespace codal
{
    /**
     * Class definition for a normalised, non-linear analog sensor, that takes the general form of a logarithmic response to a sensed value, in a potential divider.
     * Implements a base class for such a sensor, using the Steinhart-Hart equation to delineate a result.
     */
    class NonLinearAnalogSensor : public AnalogSensor
    {
        float           nominalValue;       // The value (in SI units) of a nominal position.
        float           nominalReading;     // The reading (in sensed level) at that nominal position.
        float           beta;               // The Steinhart B parameter.
        float           seriesResistor;     // the resitance (in ohms) of the associated series resistor.
        float           zeroOffset;         // A user defined "zero" point (negative asymptote).

        public:

        /**
          * Constructor.
          *
          * Creates a generic AnalogSensor.
          *
          * @param pin The pin on which to sense
          * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER
          * @param nominalValue The value (in SI units) of a nominal position.
          * @param nominalReading The raw reading from the sensor at the nominal position.
          * @param beta The Steinhart-Hart Beta constant for the device
          * @param seriesResistor The value (in ohms) of the resistor in series with the sensor.
          * @param zeroOffset Optional zero offset applied to all SI units (e.g. 273.15 for temperature sensing in C vs Kelvin).
          *
         */
        NonLinearAnalogSensor(Pin &pin, uint16_t id, float nominalValue, float nominalReading, float beta, float seriesResistor, float zeroOffset = 0.0f);

        /**
         * Updates the internal reading of the sensor. Typically called periodicaly.
         *
         * @return DEVICE_OK on success.
         */
        virtual void updateSample();

    };    
}

#endif
