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
 * Class definition for a CapTouchButtonButton.
 *
 * Represents a single, software controlled capacitative touch button on the device.
 */

#include "DeviceConfig.h"
#include "CodalDevice.h"
#include "CodalDmesg.h"
#include "CapTouchButton.h"
#include "DeviceSystemTimer.h"
#include "EventModel.h"

static bool clock_initialized;

#define CAP_TOUCH_BUTTON_UPDATE_NEEDED 0x4242

/**
 * Constructor.
 *
 * Enables software controlled capacitative touch sensing on the given pin.
 *
 * @param pin The physical pin on the device to sense.
 * @param sensor The touch sensor driver for this touch sensitive pin.
 */
CapTouchButton::CapTouchButton(DevicePin &pin, int threshold)
    : DeviceButton(pin, pin.id, DEVICE_BUTTON_ALL_EVENTS, ACTIVE_LOW, PullNone)
{
    // Disable periodic events. These will come from our TouchSensor.
    this->threshold = threshold;
    this->reading = 0;

    if (!clock_initialized)
    {
        /* Setup and enable generic clock source for PTC module. */
        struct system_gclk_chan_config gclk_chan_conf;
        system_gclk_chan_get_config_defaults(&gclk_chan_conf);
        gclk_chan_conf.source_generator = GCLK_GENERATOR_3;
        system_gclk_chan_set_config(PTC_GCLK_ID, &gclk_chan_conf);
        system_gclk_chan_enable(PTC_GCLK_ID);
        system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_PTC);

        // Generate an event every CAP_TOUCH_BUTTON_SAMPLE_PERIOD milliseconds.
        system_timer_event_every_us(CAP_TOUCH_BUTTON_SAMPLE_PERIOD * 1000, DEVICE_ID_TOUCH_SENSOR,
                                    CAP_TOUCH_BUTTON_UPDATE_NEEDED);
    }

    adafruit_ptc_get_config_default(&config);
    config.pin = pin.name;
    if (PA02 <= pin.name && pin.name <= PA07)
        config.yline = pin.name - 2;
    else if (PB02 <= pin.name && pin.name <= PB09)
        config.yline = (pin.name - 32) + 6;
    else
        device.panic(0);

    adafruit_ptc_init(PTC, &config);

    // Perform a calibraiton if necessary
    if (threshold < 0)
        calibrate();

    // Configure a periodic callback event.
    EventModel::defaultEventBus->listen(DEVICE_ID_TOUCH_SENSOR, CAP_TOUCH_BUTTON_UPDATE_NEEDED,
                                        this, &CapTouchButton::update,
                                        MESSAGE_BUS_LISTENER_IMMEDIATE);
}

void CapTouchButton::update(DeviceEvent)
{
    adafruit_ptc_start_conversion(PTC, &config);

    // this takes around 300 cycles - any context switch would be way more
    while (!adafruit_ptc_is_conversion_finished(PTC))
        ;

    this->setValue(adafruit_ptc_get_conversion_result(PTC));
}

/**
 * Determines if this button is instantenously active (i.e. pressed).
 * Internal method, use before debouncing.
 */
int CapTouchButton::buttonActive()
{
    if (status & CAP_TOUCH_BUTTON_CALIBRATING)
        return 0;

    return reading >= threshold;
}

/**
 * Estimate and apply a threshold based on the current reading of the device.
 */
void CapTouchButton::calibrate()
{
    // indicate that we're entering a calibration phase.
    // We reuse the threshold variable to track calibration progress, just to save a little memory.
    this->reading = CAP_TOUCH_BUTTON_CALIBRATION_PERIOD;
    threshold = 0;
    status |= CAP_TOUCH_BUTTON_CALIBRATING;
}

/**
 * Manually define the threshold use to detect a touch event. Any sensed value equal to or greater
 * than this value will
 * be interpreted as a touch. See getValue().
 *
 * @param threshold The threshold value to use for this touchButton.
 */
void CapTouchButton::setThreshold(int threshold)
{
    this->threshold = threshold;
}

/**
 * Determine the last reading taken from this button.
 *
 * @return the last reading taken.
 */
int CapTouchButton::getValue()
{
    return reading;
}

/**
 * Updates the record of the last reading from this button.
 */
void CapTouchButton::setValue(int reading)
{
    if (status & CAP_TOUCH_BUTTON_CALIBRATING)
    {
        // Record the highest value measured. This is our baseline.
        this->threshold = max(this->threshold, reading);
        this->reading--;

        // We've completed calibration, returnt to normal mode of operation.
        if (this->reading == 0)
        {
            int t0 = this->threshold;
            this->threshold +=
                ((this->threshold * CAP_TOUCH_BUTTON_CALIBRATION_PERCENTAGE_OFFSET) / 100) +
                CAP_TOUCH_BUTTON_CALIBRATION_LINEAR_OFFSET;
            DMESG("CapTouch Calibrated id=%d threshold=%d (max %d)", this->id, this->threshold, t0);
            status &= ~CAP_TOUCH_BUTTON_CALIBRATING;
        }

        return;
    }

    // Otherewise we're not calibrating, so simply record the result.
    this->reading = reading;
}

/**
  * Destructor for CapTouchButton, where we deregister our callback
  */
CapTouchButton::~CapTouchButton()
{
    EventModel::defaultEventBus->ignore(DEVICE_ID_TOUCH_SENSOR, CAP_TOUCH_BUTTON_UPDATE_NEEDED,
                                        this, &CapTouchButton::update);
}
