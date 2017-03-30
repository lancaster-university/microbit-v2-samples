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

#include "DeviceConfig.h"
#include "AbstractButton.h"
#include "Timer.h"
#include "EventModel.h"

using namespace codal;

/**
 * Constructor.
 *
 * Create a abstract software representation of a button.
 */
AbstractButton::AbstractButton()
{
    clickCount = 0;
}

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
int AbstractButton::isPressed()
{
    return 0;
}

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
int AbstractButton::wasPressed()
{
    uint16_t c = clickCount;
    clickCount = 0;

    return c;
}

/**
 * Destructor
 */
AbstractButton::~AbstractButton()
{
}
