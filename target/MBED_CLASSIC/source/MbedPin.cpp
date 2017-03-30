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
  * Class definition for Pin.
  *
  * Commonly represents an I/O pin on the edge connector.
  */
#include "MbedPin.h"
#include "DeviceButton.h"
#include "Timer.h"
#include "MbedTimedInterruptIn.h"
#include "DynamicPwm.h"
#include "ErrorNo.h"
#include "mbed.h"

namespace mb=mbed;
using namespace codal::mbed;

/**
  * Constructor.
  * Create a Pin instance, generally used to represent a pin on the edge connector.
  *
  * @param id the unique EventModel id of this component.
  *
  * @param name the mbed PinName for this Pin instance.
  *
  * @param capability the capabilities this Pin instance should have.
  *                   (PIN_CAPABILITY_DIGITAL, PIN_CAPABILITY_ANALOG, PIN_CAPABILITY_AD, PIN_CAPABILITY_ALL)
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
  * @endcode
  */
Pin::Pin(int id, PinName name, PinCapability capability) : codal::Pin(id, name, capability)
{
    this->pullMode = DEVICE_DEFAULT_PULLMODE;

    // Power up in a disconnected, low power state.
    // If we're unused, this is how it will stay...
    this->status = 0x00;
    this->pin = NULL;

}

/**
  * Disconnect any attached mBed IO from this pin.
  *
  * Used only when pin changes mode (i.e. Input/Output/Analog/Digital)
  */
void Pin::disconnect()
{
    // This is a bit ugly, but rarely used code.
    // It would be much better to use some polymorphism here, but the mBed I/O classes aren't arranged in an inheritance hierarchy... yet. :-)
    if (status & IO_STATUS_DIGITAL_IN)
        delete ((DigitalIn *)pin);

    if (status & IO_STATUS_DIGITAL_OUT)
        delete ((DigitalOut *)pin);

    if (status & IO_STATUS_ANALOG_IN)
        delete ((AnalogIn *)pin);

    if (status & IO_STATUS_ANALOG_OUT)
    {
        ((DynamicPwm *)pin)->release();
        delete ((DynamicPwm *)pin);
    }

    if (status & IO_STATUS_TOUCH_IN)
        delete ((DeviceButton *)pin);

    if ((status & IO_STATUS_EVENT_ON_EDGE) || (status & IO_STATUS_EVENT_PULSE_ON_EDGE))
        delete ((TimedInterruptIn *)pin);

    this->pin = NULL;
    this->status = 0;
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
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.setDigitalValue(1); // P0 is now HI
  * @endcode
  */
int Pin::setDigitalValue(int value)
{
    // Check if this pin has a digital mode...
    if(!(PIN_CAPABILITY_DIGITAL & capability))
        return DEVICE_NOT_SUPPORTED;

    // Ensure we have a valid value.
    if (value < 0 || value > 1)
        return DEVICE_INVALID_PARAMETER;

    // Move into a Digital input state if necessary.
    if (!(status & IO_STATUS_DIGITAL_OUT)){
        disconnect();
        pin = new DigitalOut(name);
        status |= IO_STATUS_DIGITAL_OUT;
    }

    // Write the value.
    ((DigitalOut *)pin)->write(value);

    return DEVICE_OK;
}

/**
  * Configures this IO pin as a digital input (if necessary) and tests its current value.
  *
  *
  * @return 1 if this input is high, 0 if input is LO, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have digital capability.
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.getDigitalValue(); // P0 is either 0 or 1;
  * @endcode
  */
int Pin::getDigitalValue()
{
    //check if this pin has a digital mode...
    if(!(PIN_CAPABILITY_DIGITAL & capability))
        return DEVICE_NOT_SUPPORTED;

    // Move into a Digital input state if necessary.
    if (!(status & (IO_STATUS_DIGITAL_IN | IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE)))
    {
        disconnect();
        pin = new DigitalIn(name, (PinMode)pullMode);
        status |= IO_STATUS_DIGITAL_IN;
    }

    if(status & (IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE))
        return ((TimedInterruptIn *)pin)->read();

    return ((DigitalIn *)pin)->read();
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
 * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
 * P0.getDigitalValue(PullUp); // P0 is either 0 or 1;
 * @endcode
 */
int Pin::getDigitalValue(PinMode pull)
{
    setPull(pull);
    return getDigitalValue();
}

int Pin::obtainAnalogChannel()
{
    // Move into an analogue input state if necessary, if we are no longer the focus of a DynamicPWM instance, allocate ourselves again!
    if (!(status & IO_STATUS_ANALOG_OUT) || !(((DynamicPwm *)pin)->getPinName() == name)){
        disconnect();
        pin = new DynamicPwm(name);
        status |= IO_STATUS_ANALOG_OUT;
    }

    return DEVICE_OK;
}

/**
  * Configures this IO pin as an analog/pwm output, and change the output value to the given level.
  *
  * @param value the level to set on the output pin, in the range 0 - 1024
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if value is out of range, or DEVICE_NOT_SUPPORTED
  *         if the given pin does not have analog capability.
  */
int Pin::setAnalogValue(int value)
{
    //check if this pin has an analogue mode...
    if(!(PIN_CAPABILITY_DIGITAL & capability))
        return DEVICE_NOT_SUPPORTED;

    //sanitise the level value
    if(value < 0 || value > DEVICE_PIN_MAX_OUTPUT)
        return DEVICE_INVALID_PARAMETER;

    float level = (float)value / float(DEVICE_PIN_MAX_OUTPUT);

    //obtain use of the DynamicPwm instance, if it has changed / configure if we do not have one
    if(obtainAnalogChannel() == DEVICE_OK)
        return ((DynamicPwm *)pin)->write(level);

    return DEVICE_OK;
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
int Pin::setServoValue(int value, int range, int center)
{
    //check if this pin has an analogue mode...
    if(!(PIN_CAPABILITY_ANALOG & capability))
        return DEVICE_NOT_SUPPORTED;

    //sanitise the servo level
    if(value < 0 || range < 1 || center < 1)
        return DEVICE_INVALID_PARAMETER;

    //clip - just in case
    if(value > DEVICE_PIN_MAX_SERVO_RANGE)
        value = DEVICE_PIN_MAX_SERVO_RANGE;

    //calculate the lower bound based on the midpoint
    int lower = (center - (range / 2)) * 1000;

    value = value * 1000;

    //add the percentage of the range based on the value between 0 and 180
    int scaled = lower + (range * (value / DEVICE_PIN_MAX_SERVO_RANGE));

    return setServoPulseUs(scaled / 1000);
}

/**
  * Configures this IO pin as an analogue input (if necessary), and samples the Pin for its analog value.
  *
  * @return the current analogue level on the pin, in the range 0 - 1024, or
  *         DEVICE_NOT_SUPPORTED if the given pin does not have analog capability.
  *
  * @code
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
  * P0.getAnalogValue(); // P0 is a value in the range of 0 - 1024
  * @endcode
  */
int Pin::getAnalogValue()
{
    //check if this pin has an analogue mode...
    if(!(PIN_CAPABILITY_ANALOG & capability))
        return DEVICE_NOT_SUPPORTED;

    // Move into an analogue input state if necessary.
    if (!(status & IO_STATUS_ANALOG_IN)){
        disconnect();
        pin = new AnalogIn(name);
        status |= IO_STATUS_ANALOG_IN;
    }

    //perform a read!
    return (((AnalogIn *)pin)->read_u16() >> 6);
}

/**
  * Determines if this IO pin is currently configured as an input.
  *
  * @return 1 if pin is an analog or digital input, 0 otherwise.
  */
int Pin::isInput()
{
    return (status & (IO_STATUS_DIGITAL_IN | IO_STATUS_ANALOG_IN)) == 0 ? 0 : 1;
}

/**
  * Determines if this IO pin is currently configured as an output.
  *
  * @return 1 if pin is an analog or digital output, 0 otherwise.
  */
int Pin::isOutput()
{
    return (status & (IO_STATUS_DIGITAL_OUT | IO_STATUS_ANALOG_OUT)) == 0 ? 0 : 1;
}

/**
  * Determines if this IO pin is currently configured for digital use.
  *
  * @return 1 if pin is digital, 0 otherwise.
  */
int Pin::isDigital()
{
    return (status & (IO_STATUS_DIGITAL_IN | IO_STATUS_DIGITAL_OUT)) == 0 ? 0 : 1;
}

/**
  * Determines if this IO pin is currently configured for analog use.
  *
  * @return 1 if pin is analog, 0 otherwise.
  */
int Pin::isAnalog()
{
    return (status & (IO_STATUS_ANALOG_IN | IO_STATUS_ANALOG_OUT)) == 0 ? 0 : 1;
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
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_ALL);
  * if(P0.isTouched())
  * {
  *     //do something!
  * }
  *
  * // subscribe to events generated by this pin!
  * bus.listen(DEVICE_ID_IO_P0, DEVICE_BUTTON_EVT_CLICK, someFunction);
  * @endcode
  */
int Pin::isTouched()
{
    //check if this pin has a touch mode...
    if(!(PIN_CAPABILITY_DIGITAL & capability))
        return DEVICE_NOT_SUPPORTED;

    // Move into a touch input state if necessary.
    if (!(status & IO_STATUS_TOUCH_IN)){
        disconnect();
        pin = new DeviceButton(*this, id);
        status |= IO_STATUS_TOUCH_IN;
    }

    return ((DeviceButton *)pin)->isPressed();
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
int Pin::setServoPulseUs(int pulseWidth)
{
    //check if this pin has an analogue mode...
    if(!(PIN_CAPABILITY_ANALOG & capability))
        return DEVICE_NOT_SUPPORTED;

    //sanitise the pulse width
    if(pulseWidth < 0)
        return DEVICE_INVALID_PARAMETER;

    //Check we still have the control over the DynamicPwm instance
    if(obtainAnalogChannel() == DEVICE_OK)
    {
        //check if the period is set to 20ms
        if(((DynamicPwm *)pin)->getPeriodUs() != DEVICE_DEFAULT_PWM_PERIOD)
            ((DynamicPwm *)pin)->setPeriodUs(DEVICE_DEFAULT_PWM_PERIOD);

        ((DynamicPwm *)pin)->pulsewidth_us(pulseWidth);
    }

    return DEVICE_OK;
}

/**
  * Configures the PWM period of the analog output to the given value.
  *
  * @param period The new period for the analog output in microseconds.
  *
  * @return DEVICE_OK on success.
  */
int Pin::setAnalogPeriodUs(int period)
{
    int ret;

    if (!(status & IO_STATUS_ANALOG_OUT))
    {
        // Drop this pin into PWM mode, but with a LOW value.
        ret = setAnalogValue(0);
        if (ret != DEVICE_OK)
            return ret;
    }

    return ((DynamicPwm *)pin)->setPeriodUs(period);
}

/**
  * Configures the PWM period of the analog output to the given value.
  *
  * @param period The new period for the analog output in milliseconds.
  *
  * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::setAnalogPeriod(int period)
{
    return setAnalogPeriodUs(period*1000);
}

/**
  * Obtains the PWM period of the analog output in microseconds.
  *
  * @return the period on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::getAnalogPeriodUs()
{
    if (!(status & IO_STATUS_ANALOG_OUT))
        return DEVICE_NOT_SUPPORTED;

    return ((DynamicPwm *)pin)->getPeriodUs();
}

/**
  * Obtains the PWM period of the analog output in milliseconds.
  *
  * @return the period on success, or DEVICE_NOT_SUPPORTED if the
  *         given pin is not configured as an analog output.
  */
int Pin::getAnalogPeriod()
{
    return getAnalogPeriodUs()/1000;
}

/**
  * Configures the pull of this pin.
  *
  * @param pull one of the mbed pull configurations: PullUp, PullDown, PullNone
  *
  * @return DEVICE_NOT_SUPPORTED if the current pin configuration is anything other
  *         than a digital input, otherwise DEVICE_OK.
  */
int Pin::setPull(PinMode pull)
{
    pullMode = pull;

    if ((status & IO_STATUS_DIGITAL_IN))
    {
        ((DigitalIn *)pin)->mode(pull);
        return DEVICE_OK;
    }

    if((status & IO_STATUS_EVENT_ON_EDGE) || (status & IO_STATUS_EVENT_PULSE_ON_EDGE))
    {
        ((TimedInterruptIn *)pin)->mode(pull);
        return DEVICE_OK;
    }

    return DEVICE_NOT_SUPPORTED;
}

/**
  * This member function manages the calculation of the timestamp of a pulse detected
  * on a pin whilst in IO_STATUS_EVENT_PULSE_ON_EDGE or IO_STATUS_EVENT_ON_EDGE modes.
  *
  * @param eventValue the event value to distribute onto the message bus.
  */
void Pin::pulseWidthEvent(int eventValue)
{
    DeviceEvent evt(id, eventValue, CREATE_ONLY);
    uint64_t now = evt.timestamp;
    uint64_t previous = ((TimedInterruptIn *)pin)->getTimestamp();

    if (previous != 0)
    {
        evt.timestamp -= previous;
        evt.fire();
    }

    ((TimedInterruptIn *)pin)->setTimestamp(now);
}

/**
  * Interrupt handler for when an rise interrupt is triggered.
  */
void Pin::onRise()
{
    if(status & IO_STATUS_EVENT_PULSE_ON_EDGE)
        pulseWidthEvent(DEVICE_PIN_EVT_PULSE_LO);

    if(status & IO_STATUS_EVENT_ON_EDGE)
        DeviceEvent(id, DEVICE_PIN_EVT_RISE);
}

/**
  * Interrupt handler for when an fall interrupt is triggered.
  */
void Pin::onFall()
{
    if(status & IO_STATUS_EVENT_PULSE_ON_EDGE)
        pulseWidthEvent(DEVICE_PIN_EVT_PULSE_HI);

    if(status & IO_STATUS_EVENT_ON_EDGE)
        DeviceEvent(id, DEVICE_PIN_EVT_FALL);
}

/**
  * This member function will construct an TimedInterruptIn instance, and configure
  * interrupts for rise and fall.
  *
  * @param eventType the specific mode used in interrupt context to determine how an
  *                  edge/rise is processed.
  *
  * @return DEVICE_OK on success
  */
int Pin::enableRiseFallEvents(int eventType)
{
    // if we are in neither of the two modes, configure pin as a TimedInterruptIn.
    if (!(status & (IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE)))
    {
        disconnect();
        pin = new TimedInterruptIn(name);

        ((TimedInterruptIn *)pin)->mode((PinMode)pullMode);
        ((TimedInterruptIn *)pin)->rise(this, &Pin::onRise);
        ((TimedInterruptIn *)pin)->fall(this, &Pin::onFall);
    }

    status &= ~(IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE);

    // set our status bits accordingly.
    if(eventType == DEVICE_PIN_EVENT_ON_EDGE)
        status |= IO_STATUS_EVENT_ON_EDGE;
    else if(eventType == DEVICE_PIN_EVENT_ON_PULSE)
        status |= IO_STATUS_EVENT_PULSE_ON_EDGE;

    return DEVICE_OK;
}

/**
  * If this pin is in a mode where the pin is generating events, it will destruct
  * the current instance attached to this Pin instance.
  *
  * @return DEVICE_OK on success.
  */
int Pin::disableEvents()
{
    if (status & (IO_STATUS_EVENT_ON_EDGE | IO_STATUS_EVENT_PULSE_ON_EDGE | IO_STATUS_TOUCH_IN))
        disconnect();

    return DEVICE_OK;
}

/**
  * Configures the events generated by this Pin instance.
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
  * Pin P0(DEVICE_ID_IO_P0, DEVICE_PIN_P0, PIN_CAPABILITY_BOTH);
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
int Pin::eventOn(int eventType)
{
    switch(eventType)
    {
        case DEVICE_PIN_EVENT_ON_EDGE:
        case DEVICE_PIN_EVENT_ON_PULSE:
            enableRiseFallEvents(eventType);
            break;

        case DEVICE_PIN_EVENT_ON_TOUCH:
            isTouched();
            break;

        case DEVICE_PIN_EVENT_NONE:
            disableEvents();
            break;

        default:
            return DEVICE_INVALID_PARAMETER;
    }

    return DEVICE_OK;
}
