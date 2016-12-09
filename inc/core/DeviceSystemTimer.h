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

#ifndef DEVICE_SYSTEM_TIMER_H
#define DEVICE_SYSTEM_TIMER_H

#include "DeviceConfig.h"
#include "DeviceComponent.h"
#include "SystemClock.h"

SystemClock* system_timer_get_instance();

void system_timer_set_instance(SystemClock* systemClock);

/**
  * Determines the time since the device was powered on.
  *
  * @return the current time since power on in milliseconds
  */
uint64_t system_timer_current_time();

/**
  * Determines the time since the device was powered on.
  *
  * @return the current time since power on in microseconds
  */
uint64_t system_timer_current_time_us();

/**
  * Fetch the system clocks' id bus id.
  *
  * @return the system clocks' id bus id.
  */
uint16_t system_timer_get_id();

/**
  * Configure an event to occur every period us.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int system_timer_event_every_us(uint64_t period, uint16_t value);

/**
  * Configure an event to occur after period us.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int system_timer_event_after_us(uint64_t period, uint16_t value);

#endif
