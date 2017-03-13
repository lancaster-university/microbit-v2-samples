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
  * Class definition for Genuino Zero IO.
  * Represents a collection of all I/O pins on the device.
  */

#include "DeviceConfig.h"
#include "CircuitPlaygroundIO.h"

/**
  * Constructor.
  *
  * Create a representation of all given I/O pins on the edge connector
  *
  * Accepts a sequence of unique ID's used to distinguish events raised
  * by MicroBitPin instances on the default EventModel.
  */
CircuitPlaygroundIO::CircuitPlaygroundIO() :
    rx (ID_PIN_RX, RXD, PIN_CAPABILITY_DIGITAL),            
    tx (ID_PIN_TX, TXD, PIN_CAPABILITY_DIGITAL),            
    sda (ID_PIN_SDA, SDA, PIN_CAPABILITY_DIGITAL),            
    scl (ID_PIN_SCL, SCL, PIN_CAPABILITY_DIGITAL),            
    d6 (ID_PIN_D6, A3, PIN_CAPABILITY_DIGITAL),            
    d9 (ID_PIN_D9, D9, PIN_CAPABILITY_DIGITAL),            
    d10 (ID_PIN_D10, D8, PIN_CAPABILITY_DIGITAL),            
    d12 (ID_PIN_D12, A4, PIN_CAPABILITY_DIGITAL),
    speaker (ID_PIN_SPEAKER, SPEAKER, PIN_CAPABILITY_AD),
    led (ID_PIN_LED, LED, PIN_CAPABILITY_DIGITAL),
    int1(ID_PIN_INT1, LIS_IRQ, PIN_CAPABILITY_DIGITAL),
    temperature(ID_PIN_THERMISTOR, SPEAKER, PIN_CAPABILITY_AD),
    light(ID_PIN_LIGHTSENSOR, LIGHT, PIN_CAPABILITY_AD),
    buttonA(ID_PIN_BUTTONA, LEFT_BUTTON, PIN_CAPABILITY_DIGITAL),
    buttonB(ID_PIN_BUTTONB, RIGHT_BUTTON, PIN_CAPABILITY_DIGITAL),
    buttonC(ID_PIN_BUTTONC, SLIDE_SWITCH, PIN_CAPABILITY_DIGITAL),
    touchDrive(ID_PIN_TOUCHDRIVE, CAPPUSH, PIN_CAPABILITY_DIGITAL)
{
}
