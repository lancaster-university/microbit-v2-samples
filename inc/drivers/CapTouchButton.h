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

#ifndef CAP_TOUCH_BUTTON_H
#define CAP_TOUCH_BUTTON_H

#include "DeviceConfig.h"
#include "DeviceButton.h"
#include "DeviceComponent.h"
#include "DeviceEvent.h"
#include "DevicePin.h"
#include "adafruit_ptc.h"


// Constants associated with TouchButton
#define CAP_TOUCH_BUTTON_CALIBRATION_PERIOD                     10          // Number of samples taken during calibraiton phase.
#define CAP_TOUCH_BUTTON_CALIBRATION_LINEAR_OFFSET              100           // Constant value added to sensed baseline to determine threshold.
#define CAP_TOUCH_BUTTON_CALIBRATION_PERCENTAGE_OFFSET          30           // Proportion (percentage) of baseline reading added to sensed baseline to determine threshold.

#define CAP_TOUCH_BUTTON_SAMPLE_PERIOD      50

// Status flags associated with a touch sensor
#define CAP_TOUCH_BUTTON_CALIBRATING            0x10


/**
  * Class definition for a CapTouchButtonButton.
  *
  * Represents a single, software controlled capacitative touch button on the device.
  */
class CapTouchButton : public DeviceButton
{
    public:

    int             threshold;              // The calibration threshold of this button
    int             reading;                // The last sample taken of this button.
    bool            active;                 // true if this button is currnelty being sensed, false otherwise.

    struct adafruit_ptc_config config;


    /**
      * Constructor.
      *
      * Enables software controlled capacitative touch sensing on the given pin.
      *
      * @param pin The physical pin on the device to sense.
      * @param sensor The touch sensor driver for this touch sensitive pin.
      * @param threshold The calibration threshold to use for this button. If undefined, auto calibration will be performed.
      */
    CapTouchButton(DevicePin &pin, int threshold = -1);

    /**
      * Estimate and apply a threshold based on the current reading of the device.
      */
    void calibrate();

    /**
      * Manually define the threshold use to detect a touch event. Any sensed value equal to or greater than this value will
      * be interpreted as a touch. See getValue().
      *
      * @param threshold The threshold value to use for this touchButton. 
      */
    void setThreshold(int threshold);

    /**
      * Determine the last reading taken from this button.
      *
      * @return the last reading taken.
      */
    int getValue();

    /**
     * Determines if this button is instantenously active (i.e. pressed).
     * Internal method, use before debouncing.
     */
    int buttonActive();

    /**
      * Destructor for CapTouchButton, where we deregister our callback
      */
    ~CapTouchButton();

private:
    /**
      * Sample capacity and store in 'reading' field
      */
    void update(DeviceEvent);

    /**
     * Updates the record of the last reading from this button.
     */
    void setValue(int reading);
};

#endif
