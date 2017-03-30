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

#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "DeviceEvent.h"
#include "Pin.h"
#include "TouchButton.h"

// Constants
#define TOUCH_SENSOR_MAX_BUTTONS        10
#define TOUCH_SENSOR_SAMPLE_PERIOD      50
#define TOUCH_SENSE_SAMPLE_MAX          1000

// Event codes associate with this touch sensor.
#define TOUCH_SENSOR_UPDATE_NEEDED      1

namespace codal
{
    class TouchButton;

    /**
      * Class definition for a TouchSensor
      *
      * Drives a number of TouchButtons ona device.
      */
    class TouchSensor : DeviceComponent
    {
        TouchButton*    buttons[TOUCH_SENSOR_MAX_BUTTONS];
        Pin             &drivePin;
        int             numberOfButtons;

        public:

        /**
          * Constructor.
          *
          * Enables software controlled capacitative touch sensing on a set of pins.
          *
          * @param pin The physical pin on the device that drives the capacitative sensing.
          * @id The ID of this component, defaults to DEVICE_ID_TOUCH_SENSOR
          */
        TouchSensor(Pin &pin, uint16_t id = DEVICE_ID_TOUCH_SENSOR);

        /**
          * Begin touch sensing on the given button
          */
        int addTouchButton(TouchButton *button);

        /**
          * Stop touch sensing on the given button
          */
        int removeTouchButton(TouchButton *button);

        /**
         * Initiate a scan of the sensors.
         */
        void onSampleEvent(DeviceEvent);

        /**
          * Destructor.
          */
        ~TouchSensor();
    };
}

#endif
