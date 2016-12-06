#ifndef CODAL_MAPPINGS_H
#define CODAL_MAPPINGS_H

#include <avr/io.h>
#define DEVICE_PIN_COUNT        3

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

struct PinMapping
{
    PinCapability capability;
    uint8_t pin;
    uint8_t bit;
    volatile uint8_t* portIn;
    volatile uint8_t* portOut;
    volatile uint8_t* configRegister;
};

static const PinMapping deviceMap[DEVICE_PIN_COUNT] = {
    {
        PIN_CAPABILITY_DIGITAL,
        13,
        5,
        &PINB,
        &PORTB,
        &DDRB
    },
    {
        PIN_CAPABILITY_DIGITAL,
        2,
        2,
        &PIND,
        &PORTD,
        &DDRD
    },
    {
        PIN_CAPABILITY_AD,
        14,
        0,
        &PINC,
        &PORTC,
        &DDRC
    }
};

#endif
