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
#define DEVICE_ID_TIMER_1             14
#define DEVICE_ID_SCHEDULER           15

#define DEVICE_ID_IO_P0               100                       // IDs 100-227 are reserved for I/O Pin IDs.

#define DEVICE_ID_MESSAGE_BUS_LISTENER            1021          // Message bus indication that a handler for a given ID has been registered.
#define DEVICE_ID_NOTIFY_ONE                      1022          // Notfication channel, for general purpose synchronisation
#define DEVICE_ID_NOTIFY                          1023          // Notfication channel, for general purpose synchronisation

// Universal flags used as part of the status field
#define DEVICE_COMPONENT_RUNNING                0x10000000

#define DEVICE_COMPONENT_STATUS_SYSTEM_TICK     0x20000000
#define DEVICE_COMPONENT_STATUS_IDLE_TICK       0x40000000

/**
  * Class definition for DeviceComponent.
  *
  * All components should inherit from this class.
  *
  * If a component requires regular updates, then that component can be added to the
  * to the systemTick and/or idleTick queues. This provides a simple, extensible mechanism
  * for code that requires periodic/occasional background processing but does not warrant
  * the complexity of maintaining its own thread.
  *
  * Two levels of support are available.
  *
  * systemTick() provides a periodic callback during the
  * codal device's system timer interrupt. This provides a guaranteed periodic callback, but in interrupt context
  * and is suitable for code with lightweight processing requirements, but strict time constraints.
  *
  * idleTick() provides a periodic callback whenever the scheduler is idle. This provides occasional, callbacks
  * in the main thread context, but with no guarantees of frequency. This is suitable for non-urgent background tasks.
  *
  * Components wishing to use these facilities should override the systemTick and/or idleTick functions defined here, and
  * register their components using system_timer_add_component() fiber_add_idle_component() respectively.
  *
  */
class DeviceComponent
{

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
      * If you have added your component to the idle or system tick component arrays,
      * you must remember to remove your component from them if your component is destructed.
      */
    virtual ~DeviceComponent()
    {
        removeComponent();
    }

    private:
    void addComponent()
    {
        uint8_t i = 0;

        // iterate through our list to ensure no duplicate entries.
        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(components[i] == this)
                return;

            i++;
        }

        i = 0;

        // iterate through our list until an empty space is found.
        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(components[i] == NULL)
            {
                components[i] = this;
                return;
            }

            i++;
        }
    }

    void removeComponent()
    {
        uint8_t i = 0;

        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(components[i] == this)
            {
                components[i] = NULL;
                return;
            }

            i++;
        }
    }
};

#endif
