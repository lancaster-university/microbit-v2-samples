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
 * Class definition for a TouchButtonButton.
 *
 * Represents a single, software controlled capacitative touch button on the device.
 */

#include "DeviceConfig.h"
#include "TouchButton.h"
#include "Timer.h"
#include "EventModel.h"

using namespace codal;

/**
 * Constructor.
 *
 * Enables software controlled capacitative touch sensing on the given pin.
 *
 * @param pin The physical pin on the device to sense.
 * @param sensor The touch sensor driver for this touch sensitive pin.
 */
TouchButton::TouchButton(Pin &pin, TouchSensor &sensor, int threshold) : DeviceButton(pin, pin.id, DEVICE_BUTTON_ALL_EVENTS, ACTIVE_LOW, PullNone), touchSensor(sensor)
{
    // Disable periodic events. These will come from our TouchSensor.
    this->threshold = threshold;
    this->reading = 0;

    // register ourselves with the sensor
    touchSensor.addTouchButton(this);

    // Perform a calibraiton if necessary
    if (threshold < 0)
        calibrate();
}

/**
 * Determines if this button is instantenously active (i.e. pressed).
 * Internal method, use before debouncing.
 */
int TouchButton::buttonActive()
{
    if (status & TOUCH_BUTTON_CALIBRATING)
        return 0;

    return reading >= threshold;
}

/**
 * Estimate and apply a threshold based on the current reading of the device.
 */
void TouchButton::calibrate()
{
    // indicate that we're entering a calibration phase.
    // We reuse the threshold variable to track calibration progress, just to save a little memory.
    this->reading = TOUCH_BUTTON_CALIBRATION_PERIOD;
    threshold = 0;
    status |= TOUCH_BUTTON_CALIBRATING;
}

/**
 * Manually define the threshold use to detect a touch event. Any sensed value equal to or greater than this value will
 * be interpreted as a touch. See getValue().
 *
 * @param threshold The threshold value to use for this touchButton.
 */
void TouchButton::setThreshold(int threshold)
{
    this->threshold = threshold;
}

/**
 * Determine the last reading taken from this button.
 *
 * @return the last reading taken.
 */
int TouchButton::getValue()
{
    return reading;
}

/**
 * Updates the record of the last reading from this button.
 */
void TouchButton::setValue(int reading)
{
    if (status & TOUCH_BUTTON_CALIBRATING)
    {
        // Record the highest value measured. This is our baseline.
        this->threshold = max(this->threshold, reading);
        this->reading--;

        // We've completed calibration, returnt to normal mode of operation.
        if (this->reading == 0)
        {
            this->threshold += ((this->threshold * 5) / 100) + TOUCH_BUTTON_CALIBRATION_LINEAR_OFFSET;
            status &= ~TOUCH_BUTTON_CALIBRATING;
        }

        return;
    }

    // Otherewise we're not calibrating, so simply record the result.
    this->reading = reading;
}


/**
  * Destructor for TouchButton, where we deregister this touch button with our sensor...
  */
TouchButton::~TouchButton()
{
    touchSensor.removeTouchButton(this);
}
