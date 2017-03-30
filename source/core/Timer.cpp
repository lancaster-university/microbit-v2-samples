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
  * Definitions for the Device system timer.
  *
  * This module provides:
  *
  * 1) a concept of global system time since power up
  * 2) a simple periodic multiplexing API for the underlying mbed implementation.
  *
  * The latter is useful to avoid costs associated with multiple mbed Ticker instances
  * in codal components, as each incurs a significant additional RAM overhead (circa 80 bytes).
  */
#include "DeviceConfig.h"
#include "Timer.h"
#include "ErrorNo.h"


// System timer.
static codal::Timer* system_timer = NULL;

codal::Timer* system_timer_get_instance()
{
    return system_timer;
}

void system_timer_set_instance(codal::Timer* systemTimer)
{
    system_timer = systemTimer;
}

/**
  * Determines the time since the device was powered on.
  *
  * @return the current time since power on in milliseconds
  */
uint64_t system_timer_current_time()
{
    if(system_timer == NULL)
        return 0;

    return system_timer->getTime();
}

/**
  * Determines the time since the device was powered on.
  *
  * @return the current time since power on in microseconds
  */
uint64_t system_timer_current_time_us()
{
    if(system_timer == NULL)
        return 0;

    return system_timer->getTimeUs();
}

/**
  * Configure an event to occur every period us.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int system_timer_event_every_us(uint64_t period, uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->eventEveryUs(period, id, value);
}

/**
  * Configure an event to occur after period us.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int system_timer_event_after_us(uint64_t period, uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->eventAfterUs(period, id, value);
}
