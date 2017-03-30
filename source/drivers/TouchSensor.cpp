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

#include "TouchSensor.h"
#include "DeviceEvent.h"
#include "DeviceFiber.h"
#include "Timer.h"

using namespace codal;

/**
 * Constructor.
 *
 * Enables software controlled capacitative touch sensing on a set of pins.
 *
 * @param pin The physical pin on the device that drives the capacitative sensing.
 * @id The ID of this component, defaults to DEVICE_ID_TOUCH_SENSOR
 */
TouchSensor::TouchSensor(Pin &pin, uint16_t id) : drivePin(pin)
{
    this->id = id;
    this->numberOfButtons = 0;

    // Initialise output drive low (to drain any residual charge before sampling begins).
    drivePin.setDigitalValue(0);

    // Clear list off attached buttons.
    for (int i=0; i<TOUCH_SENSOR_MAX_BUTTONS; i++)
        buttons[i] = NULL;

    // Configure a periodic callback event.
    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->listen(id, TOUCH_SENSOR_UPDATE_NEEDED, this, &TouchSensor::onSampleEvent, MESSAGE_BUS_LISTENER_IMMEDIATE);

    // Generate an event every TOUCH_SENSOR_SAMPLE_PERIOD milliseconds.
    system_timer_event_every_us(TOUCH_SENSOR_SAMPLE_PERIOD * 1000, id, TOUCH_SENSOR_UPDATE_NEEDED);
}

/**
 * Begin touch sensing on the given button
 */
int TouchSensor::addTouchButton(TouchButton *button)
{
    // if our limit of buttons is reached, then there's nothing more to do.
    if (numberOfButtons == TOUCH_SENSOR_MAX_BUTTONS)
        return DEVICE_NO_RESOURCES;

    // Protect against duplicate buttons from being added.
    for (int i=0; i<numberOfButtons; i++)
        if (buttons[i] == button)
            return DEVICE_INVALID_PARAMETER;

    // Otherwise, add this new button to the end of the list.
    buttons[numberOfButtons] = button;
    numberOfButtons++;

    // Put the button into input mode.
    button->_pin.getDigitalValue();

    return DEVICE_OK;
}

/**
 * Stop touch sensing on the given button
 */
int TouchSensor::removeTouchButton(TouchButton *button)
{
    // First, find the button, if we have it.
    for (int i=0; i<numberOfButtons; i++)
    {
        if (buttons[i] == button)
        {
            // replace this entry with the last in the list, to ensure the list remains contiguous.
            buttons[i] = buttons[numberOfButtons];
            numberOfButtons--;

            return DEVICE_OK;
        }
    }

    return DEVICE_INVALID_PARAMETER;
}

/**
  * Initiate a scan of the sensors.
  */
void TouchSensor::onSampleEvent(DeviceEvent)
{
    int cycles = 0;
    int activeSensors = 0;

    // Drain any residual charge on the receiver pins.
    // TODO: Move this to a platform specific library function (DevicePin).
    for (int i=0; i<numberOfButtons; i++)
    {
        PORT->Group[0].OUTCLR.reg = 1 << buttons[i]->_pin.name;
        PORT->Group[0].DIRSET.reg = 1 << buttons[i]->_pin.name;
        PORT->Group[0].DIRCLR.reg = 1 << buttons[i]->_pin.name;
        buttons[i]->active = true;
    }

    // Wait for any charge to drain.
    // TODO: minimise this value.
    wait_ms(1);

    // raise the drive pin, and start testing the receiver pins...
    drivePin.setDigitalValue(1);

    while(1)
    {
        activeSensors = 0;

        for (int i=0; i<numberOfButtons; i++)
        {
            if (buttons[i]->active)
            {
                if(buttons[i]->_pin.getDigitalValue() == 1 || cycles >= (buttons[i]->threshold))
                {
                    buttons[i]->active = false;
                    buttons[i]->setValue(cycles);
                }
                activeSensors++;
            }
        }

        cycles += numberOfButtons;

        if (activeSensors == 0 || cycles > TOUCH_SENSE_SAMPLE_MAX)
            break;
    }

    drivePin.setDigitalValue(0);
}

/**
 * Destructor.
 */
TouchSensor::~TouchSensor()
{
    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->ignore(id, TOUCH_SENSOR_UPDATE_NEEDED, this, &TouchSensor::onSampleEvent);
}
