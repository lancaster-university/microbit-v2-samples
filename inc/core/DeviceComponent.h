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

#ifndef DEVICE_COMPONENT_H
#define DEVICE_COMPONENT_H

#include "DeviceConfig.h"
#include "ErrorNo.h"

// Enumeration of core components.
#define DEVICE_ID_BUTTON_A            1                         // IDs used by commonly used components. Used by convention.
#define DEVICE_ID_BUTTON_B            2
#define DEVICE_ID_BUTTON_AB           3
#define DEVICE_ID_BUTTON_RESET        4
#define DEVICE_ID_ACCELEROMETER       5
#define DEVICE_ID_COMPASS             6
#define DEVICE_ID_DISPLAY             7
#define DEVICE_ID_THERMOMETER         8
#define DEVICE_ID_RADIO               9
#define DEVICE_ID_RADIO_DATA_READY    10
#define DEVICE_ID_MULTIBUTTON_ATTACH  11
#define DEVICE_ID_SERIAL              12
#define DEVICE_ID_GESTURE             13
#define DEVICE_ID_SYSTEM_TIMER        14
#define DEVICE_ID_SCHEDULER           15
#define DEVICE_ID_COMPONENT           16

#define DEVICE_ID_IO_P0               100                       // IDs 100-227 are reserved for I/O Pin IDs.

#define DEVICE_ID_MESSAGE_BUS_LISTENER            1021          // Message bus indication that a handler for a given ID has been registered.
#define DEVICE_ID_NOTIFY_ONE                      1022          // Notfication channel, for general purpose synchronisation
#define DEVICE_ID_NOTIFY                          1023          // Notfication channel, for general purpose synchronisation

// Universal flags used as part of the status field
#define DEVICE_COMPONENT_RUNNING                0x1000

#define DEVICE_COMPONENT_STATUS_SYSTEM_TICK     0x2000
#define DEVICE_COMPONENT_STATUS_IDLE_TICK       0x4000

#define DEVICE_COMPONENT_LISTENER_CONFIGURED    0x01

#define DEVICE_COMPONENT_EVT_TICK               1

/**
  * Class definition for DeviceComponent.
  *
  * All components should inherit from this class.
  *
  * If a component requires regular updates, then that component can be added to the
  * to the periodicCallback and/or idleCallback queues. This provides a simple, extensible mechanism
  * for code that requires periodic/occasional background processing but does not warrant
  * the complexity of maintaining its own thread.
  *
  * Two levels of support are available.
  *
  * periodicCallback() provides a periodic callback during the
  * codal device's system timer interrupt. This provides a guaranteed periodic callback, but in interrupt context
  * and is suitable for code with lightweight processing requirements, but strict time constraints.
  *
  * idleCallback() provides a periodic callback whenever the scheduler is idle. This provides occasional, callbacks
  * in the main thread context, but with no guarantees of frequency. This is suitable for non-urgent background tasks.
  *
  * Components wishing to use these facilities should override the periodicCallback and/or idleCallback functions defined here, and
  * register their components using system_timer_add_component() fiber_add_idle_component() respectively.
  *
  */
class DeviceComponent
{
    static uint8_t configuration;

    /**
      * Adds the current DeviceComponent instance to our array of components.
      */
    void addComponent();

    /**
      * Removes the current DeviceComponent instance from our array of components.
      */
    void removeComponent();

    public:

    static DeviceComponent* components[DEVICE_COMPONENT_COUNT];

    uint16_t id;                    // Event Bus ID of this component
    uint16_t status;                // Component defined state.

    /**
      * The default constructor of a DeviceComponent
      */
    DeviceComponent()
    {
        this->id = 0;
        this->status = 0;

        addComponent();
    }

    DeviceComponent(uint16_t id, uint16_t status)
    {
        this->id = id;
        this->status = status;

        addComponent();
    }

    /**
      * Implement this function to receive a function call after the devices'
      * device model has been instantiated.
      */
    virtual int init() { return DEVICE_NOT_SUPPORTED; }

    /**
      * Implement this function to receive a callback every SCHEDULER_TICK_PERIOD_MS.
      */
    virtual void periodicCallback() {}

    /**
      * Implement this function to receive a callback when the device is idling.
      */
    virtual void idleCallback() {}

    /**
      * If you have added your component to the idle or system tick component arrays,
      * you must remember to remove your component from them if your component is destructed.
      */
    virtual ~DeviceComponent()
    {
        removeComponent();
    }
};

#endif
