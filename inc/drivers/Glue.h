/*
The MIT License (MIT)

Copyright (c) 2016 Lancaster University, UK.

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

#ifndef CODAL_DRIVER_GLUE_H
#define CODAL_DRIVER_GLUE_H

#include "mbed.h"
#include "DeviceConfig.h"

//
// Processors and boards sometime contains "features" or even "silicon bugs" that occasionally means they need special treatement, or workarounds.
// here we weakly define a set of functions for known cases of this nature. by default, the codal calls these functions, nothing will happen.
// However, they provide "hooks" for a specific target to override shoudl they need to do so.
//

//
// Explicitly disable ADC behaviour. 
// known uses: nrf51822 PAN 3, details of which can be found here: https://www.nordicsemi.com/eng/nordic/download_resource/24634/5/88440387
//
void analogin_disable() __attribute__((weak));

//
// Explicitly redirect a PWM channel to given pin.
// known uses: nrf51822 has a limited set of GPIOTE channels that do not normally permit reuse. Used by DynamicPWM.
//
void pwmout_redirect(PinName pin, pwmout_t *obj) __attribute__((weak));

#endif
