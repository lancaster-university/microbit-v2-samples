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

#ifndef LINEAR_ANALOG_SENSOR_H
#define LINEAR_ANALOG_SENSOR_H

#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "Pin.h"
#include "DeviceEvent.h"
#include "AnalogSensor.h"


namespace codal
{
    /**
     * Class definition for a normalised, linear analog sensor, that takes the general form of a linear response between sensed value and result.
     * Given an input range and an output range, automatically scales sampled data into the given output range.
     */
    class LinearAnalogSensor : public AnalogSensor
    {
        uint16_t     inputFloor;        // The minimum level in the input range.
        float        outputFloor;       // The minimum level in the output range.
        float        conversionFactor;  // no of output units per input unit.

        public:

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
        LinearAnalogSensor(Pin &pin, uint16_t id, uint16_t sampleFloor, uint16_t sampleCeiling, float valueFloor = 0.0f, float valueCeiling = 1023.0f);

        /**
         * Updates the internal reading of the sensor. Typically called periodicaly.
         *
         * @return DEVICE_OK on success.
         */
        virtual void updateSample();

    };
}

#endif
