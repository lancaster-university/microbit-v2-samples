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

#ifndef CIRCUIT_PLAYGROUND_H
#define CIRCUIT_PLAYGROUND_H

#include "mbed.h"

#include "DeviceConfig.h"
#include "DeviceHeapAllocator.h"
#include "CodalDevice.h"
#include "ErrorNo.h"
#include "MbedTimer.h"
#include "Matrix4.h"
#include "CodalCompat.h"
#include "DeviceComponent.h"
#include "ManagedType.h"
#include "ManagedString.h"
#include "DeviceEvent.h"
#include "NotifyEvents.h"

#include "DeviceButton.h"
#include "MultiButton.h"
#include "MbedPin.h"
#include "MbedI2C.h"
#include "LIS3DH.h"
#include "LinearAnalogSensor.h"
#include "NonLinearAnalogSensor.h"
#include "TouchSensor.h"

#include "MbedSerial.h"
#include "CircuitPlaygroundIO.h"

#include "DeviceFiber.h"
#include "DeviceMessageBus.h"

using namespace codal;
/**
  * Class definition for a CircuitPlayground device.
  *
  * Represents the device as a whole, and includes member variables that represent various device drivers
  * used to control aspects of the device.
  */
class CircuitPlayground
{
    private:

    /**
      * A listener to perform actions as a result of Message Bus reflection.
      *
      * In some cases we want to perform lazy instantiation of components, such as
      * the compass and the accelerometer, where we only want to add them to the idle
      * fiber when someone has the intention of using these components.
      */
    void                        onListenerRegisteredEvent(DeviceEvent evt);
    uint8_t                     status;

    public:

    DeviceMessageBus            messageBus;
    codal::mbed::Timer          timer;
    codal::mbed::Serial         serial;
    CircuitPlaygroundIO         io;
    DeviceButton                buttonA;
    DeviceButton                buttonB;
    DeviceButton                buttonC;
    MultiButton                 buttonAB;

    codal::mbed::I2C            i2c;
    LIS3DH                      accelerometer;
    NonLinearAnalogSensor       thermometer;
    AnalogSensor                lightSensor;
    TouchSensor                 touchSensor;

    /**
      * Constructor.
      *
      * Create a representation of a Genuino Zero device, which includes member variables
      * that represent various device drivers used to control aspects of the board.
      */
    CircuitPlayground();

    /**
     * Delay execution for the given amount of time.
     *
     * If the scheduler is running, this will deschedule the current fiber and perform
     * a power efficient, concurrent sleep operation.
     *
     * If the scheduler is disabled or we're running in an interrupt context, this
     * will revert to a busy wait.
     *
     * Alternatively: wait, wait_ms, wait_us can be used which will perform a blocking sleep
     * operation.
     *
     * @param milliseconds the amount of time, in ms, to wait for. This number cannot be negative.
     *
     * @return MICROBIT_OK on success, MICROBIT_INVALID_PARAMETER milliseconds is less than zero.
     *
     */
    void sleep(uint32_t milliseconds);
};


/**
 * Delay execution for the given amount of time.
 *
 * If the scheduler is running, this will deschedule the current fiber and perform
 * a power efficient, concurrent sleep operation.
 *
 * If the scheduler is disabled or we're running in an interrupt context, this
 * will revert to a busy wait.
 *
 * Alternatively: wait, wait_ms, wait_us can be used which will perform a blocking sleep
 * operation.
 *
 * @param milliseconds the amount of time, in ms, to wait for. This number cannot be negative.
 *
 * @return MICROBIT_OK on success, MICROBIT_INVALID_PARAMETER milliseconds is less than zero.
 *
 */
inline void CircuitPlayground::sleep(uint32_t milliseconds)
{
    fiber_sleep(milliseconds);
}

/**
  * A listener to perform actions as a result of Message Bus reflection.
  *
  * In some cases we want to perform lazy instantiation of components, such as
  * the compass and the accelerometer, where we only want to add them to the idle
  * fiber when someone has the intention of using these components.
  */
void onListenerRegisteredEvent(DeviceEvent evt);

#endif
