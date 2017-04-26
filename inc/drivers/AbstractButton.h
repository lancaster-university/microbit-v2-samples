/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University.

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

#ifndef ABSTRACT_BUTTON_H
#define ABSTRACT_BUTTON_H

#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "DeviceEvent.h"

#define DEVICE_BUTTON_EVT_DOWN                1
#define DEVICE_BUTTON_EVT_UP                  2
#define DEVICE_BUTTON_EVT_CLICK               3
#define DEVICE_BUTTON_EVT_LONG_CLICK          4
#define DEVICE_BUTTON_EVT_HOLD                5
#define DEVICE_BUTTON_EVT_DOUBLE_CLICK        6

#define DEVICE_BUTTON_LONG_CLICK_TIME         1000
#define DEVICE_BUTTON_HOLD_TIME               1500

#define DEVICE_BUTTON_STATE                   0x01
#define DEVICE_BUTTON_STATE_HOLD_TRIGGERED    0x02
#define DEVICE_BUTTON_STATE_CLICK             0x04
#define DEVICE_BUTTON_STATE_LONG_CLICK        0x08

#define DEVICE_BUTTON_SIGMA_MIN               0
#define DEVICE_BUTTON_SIGMA_MAX               12
#define DEVICE_BUTTON_SIGMA_THRESH_HI         8
#define DEVICE_BUTTON_SIGMA_THRESH_LO         2
#define DEVICE_BUTTON_DOUBLE_CLICK_THRESH     50

enum DeviceButtonEventConfiguration
{
    DEVICE_BUTTON_SIMPLE_EVENTS,
    DEVICE_BUTTON_ALL_EVENTS
};

enum DeviceButtonPolarity
{
    ACTIVE_LOW = 0,
    ACTIVE_HIGH = 1
};

namespace codal
{
    /**
      * Class definition for Device Button.
      *
      * Represents a single, generic button on the device.
      */
    class AbstractButton : public DeviceComponent
    {
        public:

        uint16_t clickCount;

        /**
          * Constructor.
          *
          * Create a abstract software representation of a button.
          */
        AbstractButton();

        /**
          * Tests if this Button is currently pressed.
          *
          * @code
          * if(buttonA.isPressed())
          *     display.scroll("Pressed!");
          * @endcode
          *
          * @return 1 if this button is pressed, 0 otherwise.
          */
        virtual int isPressed();

        /**
          * Determines if this button has been pressed.
          *
          * @code
          * if(buttonA.wasPressed())
          *     display.scroll("Pressed!");
          * @endcode
          *
          * @return the number of time this button has been pressed since the last time wasPressed() has been called.
          */
        int wasPressed();

        /**
         * Destructor
         */
        ~AbstractButton();
    };
}

#endif
