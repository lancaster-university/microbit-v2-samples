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

#ifndef CODAL_PIN_H
#define CODAL_PIN_H

#include <Pin.h>
#include "DeviceConfig.h"
#include "device_types.h"
#include "DeviceComponent.h"
                                                        // Status Field flags...
#define IO_STATUS_DIGITAL_IN                0x01        // Pin is configured as a digital input, with no pull up.
#define IO_STATUS_DIGITAL_OUT               0x02        // Pin is configured as a digital output
#define IO_STATUS_ANALOG_IN                 0x04        // Pin is Analog in
#define IO_STATUS_ANALOG_OUT                0x08        // Pin is Analog out
#define IO_STATUS_TOUCH_IN                  0x10        // Pin is a makey-makey style touch sensor
#define IO_STATUS_EVENT_ON_EDGE             0x20        // Pin will generate events on pin change
#define IO_STATUS_EVENT_PULSE_ON_EDGE       0x40        // Pin will generate events on pin change

#define DEVICE_PIN_MAX_OUTPUT             1023

#define DEVICE_PIN_MAX_SERVO_RANGE        180
#define DEVICE_PIN_DEFAULT_SERVO_RANGE    2000
#define DEVICE_PIN_DEFAULT_SERVO_CENTER   1500

#define DEVICE_PIN_EVENT_NONE             0
#define DEVICE_PIN_EVENT_ON_EDGE          1
#define DEVICE_PIN_EVENT_ON_PULSE         2
#define DEVICE_PIN_EVENT_ON_TOUCH         3

#define DEVICE_PIN_EVT_RISE               2
#define DEVICE_PIN_EVT_FALL               3
#define DEVICE_PIN_EVT_PULSE_HI           4
#define DEVICE_PIN_EVT_PULSE_LO           5

namespace codal
{
    using namespace codal;
    /**
      * Pin capabilities enum.
      * Used to determine the capabilities of each Pin as some can only be digital, or can be both digital and analogue.
      */
    enum PinCapability{
        PIN_CAPABILITY_DIGITAL = 0x01,
        PIN_CAPABILITY_ANALOG = 0x02,
        PIN_CAPABILITY_AD = PIN_CAPABILITY_DIGITAL | PIN_CAPABILITY_ANALOG,
        PIN_CAPABILITY_ALL = PIN_CAPABILITY_DIGITAL | PIN_CAPABILITY_ANALOG
    };

    /**
      * Class definition for DevicePin.
      *
      * Commonly represents an I/O pin on the edge connector.
      */
    class Pin : public DeviceComponent
    {
        protected:
        PinCapability capability;
        PinMode pullMode;

        public:

        // the name of this pin, a number that maps to hardware.
        PinName name;

        /**
          * Constructor.
          * Create a DevicePin instance, generally used to represent a pin on the edge connector.
          *
          * @param id the unique EventModel id of this component.
          *
          * @param name the mbed PinName for this DevicePin instance.
          *
          * @param capability the capabilities this DevicePin instance should have.
          *                   (PIN_CAPABILITY_DIGITAL, PIN_CAPABILITY_ANALOG, PIN_CAPABILITY_AD, PIN_CAPABILITY_ALL)
          *
          * @code
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
          * @endcode
          */
        Pin(int id, PinName name, PinCapability capability)
        {
            this->id = id;
            this->name = name;
            this->capability = capability;
        }

        /**
          * Configures this IO pin as a digital output (if necessary) and sets the pin to 'value'.
          *
          * @param value 0 (LO) or 1 (HI)
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
          *         if the given pin does not have digital capability.
          *
          * @code
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
          * P0.setDigitalValue(1); // P0 is now HI
          * @endcode
          */
        virtual int setDigitalValue(int value)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as a digital input (if necessary) and tests its current value.
          *
          *
          * @return 1 if this input is high, 0 if input is LO, or DEVICE_NOT_SUPPORTED
          *         if the given pin does not have digital capability.
          *
          * @code
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
          * P0.getDigitalValue(); // P0 is either 0 or 1;
          * @endcode
          */
        virtual int getDigitalValue()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as a digital input with the specified internal pull-up/pull-down configuraiton (if necessary) and tests its current value.
          *
          * @param pull one of the mbed pull configurations: PullUp, PullDown, PullNone
          *
          * @return 1 if this input is high, 0 if input is LO, or DEVICE_NOT_SUPPORTED
          *         if the given pin does not have digital capability.
          *
          * @code
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
          * P0.getDigitalValue(PullUp); // P0 is either 0 or 1;
          * @endcode
          */
        virtual int getDigitalValue(PinMode pull)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as an analog/pwm output, and change the output value to the given level.
          *
          * @param value the level to set on the output pin, in the range 0 - 1024
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
          *         if the given pin does not have analog capability.
          */
        virtual int setAnalogValue(int value)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as an analog/pwm output (if necessary) and configures the period to be 20ms,
          * with a duty cycle between 500 us and 2500 us.
          *
          * A value of 180 sets the duty cycle to be 2500us, and a value of 0 sets the duty cycle to be 500us by default.
          *
          * This range can be modified to fine tune, and also tolerate different servos.
          *
          * @param value the level to set on the output pin, in the range 0 - 180.
          *
          * @param range which gives the span of possible values the i.e. the lower and upper bounds (center +/- range/2). Defaults to DEVICE_PIN_DEFAULT_SERVO_RANGE.
          *
          * @param center the center point from which to calculate the lower and upper bounds. Defaults to DEVICE_PIN_DEFAULT_SERVO_CENTER
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
          *         if the given pin does not have analog capability.
          */
        virtual int setServoValue(int value, int range = DEVICE_PIN_DEFAULT_SERVO_RANGE, int center = DEVICE_PIN_DEFAULT_SERVO_CENTER)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as an analogue input (if necessary), and samples the Pin for its analog value.
          *
          * @return the current analogue level on the pin, in the range 0 - 1024, or
          *         DEVICE_NOT_SUPPORTED if the given pin does not have analog capability.
          *
          * @code
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
          * P0.getAnalogValue(); // P0 is a value in the range of 0 - 1024
          * @endcode
          */
        virtual int getAnalogValue()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Determines if this IO pin is currently configured as an input.
          *
          * @return 1 if pin is an analog or digital input, 0 otherwise.
          */
        virtual int isInput()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Determines if this IO pin is currently configured as an output.
          *
          * @return 1 if pin is an analog or digital output, 0 otherwise.
          */
        virtual int isOutput()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Determines if this IO pin is currently configured for digital use.
          *
          * @return 1 if pin is digital, 0 otherwise.
          */
        virtual int isDigital()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Determines if this IO pin is currently configured for analog use.
          *
          * @return 1 if pin is analog, 0 otherwise.
          */
        virtual int isAnalog()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as a "makey makey" style touch sensor (if necessary)
          * and tests its current debounced state.
          *
          * Users can also subscribe to DeviceButton events generated from this pin.
          *
          * @return 1 if pin is touched, 0 if not, or DEVICE_NOT_SUPPORTED if this pin does not support touch capability.
          *
          * @code
          * DeviceMessageBus bus;
          *
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
          * if(P0.isTouched())
          * {
          *     //do something!
          * }
          *
          * // subscribe to events generated by this pin!
          * bus.listen(DEVICE_ID_IO_P0, DEVICE_BUTTON_EVT_CLICK, someFunction);
          * @endcode
          */
        virtual int isTouched()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures this IO pin as an analog/pwm output if it isn't already, configures the period to be 20ms,
          * and sets the pulse width, based on the value it is given.
          *
          * @param pulseWidth the desired pulse width in microseconds.
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
          *         if the given pin does not have analog capability.
          */
        virtual int setServoPulseUs(int pulseWidth)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures the PWM period of the analog output to the given value.
          *
          * @param period The new period for the analog output in milliseconds.
          *
          * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
          *         given pin is not configured as an analog output.
          */
        virtual int setAnalogPeriod(int period)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures the PWM period of the analog output to the given value.
          *
          * @param period The new period for the analog output in microseconds.
          *
          * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
          *         given pin is not configured as an analog output.
          */
        virtual int setAnalogPeriodUs(int period)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Obtains the PWM period of the analog output in microseconds.
          *
          * @return the period on success, or DEVICE_NOT_SUPPORTED if the
          *         given pin is not configured as an analog output.
          */
        virtual int getAnalogPeriodUs()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Obtains the PWM period of the analog output in milliseconds.
          *
          * @return the period on success, or DEVICE_NOT_SUPPORTED if the
          *         given pin is not configured as an analog output.
          */
        virtual int getAnalogPeriod()
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures the pull of this pin.
          *
          * @param pull one of the mbed pull configurations: PullUp, PullDown, PullNone
          *
          * @return DEVICE_NOT_SUPPORTED if the current pin configuration is anything other
          *         than a digital input, otherwise DEVICE_OK.
          */
        virtual int setPull(PinMode pull)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
          * Configures the events generated by this DevicePin instance.
          *
          * DEVICE_PIN_EVENT_ON_EDGE - Configures this pin to a digital input, and generates events whenever a rise/fall is detected on this pin. (DEVICE_PIN_EVT_RISE, DEVICE_PIN_EVT_FALL)
          * DEVICE_PIN_EVENT_ON_PULSE - Configures this pin to a digital input, and generates events where the timestamp is the duration that this pin was either HI or LO. (DEVICE_PIN_EVT_PULSE_HI, DEVICE_PIN_EVT_PULSE_LO)
          * DEVICE_PIN_EVENT_ON_TOUCH - Configures this pin as a makey makey style touch sensor, in the form of a DeviceButton. Normal button events will be generated using the ID of this pin.
          * DEVICE_PIN_EVENT_NONE - Disables events for this pin.
          *
          * @param eventType One of: DEVICE_PIN_EVENT_ON_EDGE, DEVICE_PIN_EVENT_ON_PULSE, DEVICE_PIN_EVENT_ON_TOUCH, DEVICE_PIN_EVENT_NONE
          *
          * @code
          * DeviceMessageBus bus;
          *
          * DevicePin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
          * P0.eventOn(DEVICE_PIN_EVENT_ON_PULSE);
          *
          * void onPulse(DeviceEvent evt)
          * {
          *     int duration = evt.timestamp;
          * }
          *
          * bus.listen(DEVICE_ID_IO_P0, DEVICE_PIN_EVT_PULSE_HI, onPulse, MESSAGE_BUS_LISTENER_IMMEDIATE)
          * @endcode
          *
          * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the given eventype does not match
          *
          * @note In the DEVICE_PIN_EVENT_ON_PULSE mode, the smallest pulse that was reliably detected was 85us, around 5khz. If more precision is required,
          *       please use the InterruptIn class supplied by ARM mbed.
          */
        virtual int eventOn(int eventType)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }
    };

}

#endif
