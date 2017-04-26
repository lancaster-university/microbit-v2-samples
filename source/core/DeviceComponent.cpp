#include "DeviceComponent.h"
#include "DeviceFiber.h"
#include "EventModel.h"
#include "Timer.h"

using namespace codal;

DeviceComponent* DeviceComponent::components[DEVICE_COMPONENT_COUNT];

uint8_t DeviceComponent::configuration = 0;

/**
  * The periodic callback for all components.
  */
void component_callback(DeviceEvent evt)
{
    uint8_t i = 0;

    if(evt.value == DEVICE_COMPONENT_EVT_SYSTEM_TICK)
    {
        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(DeviceComponent::components[i] && DeviceComponent::components[i]->status & DEVICE_COMPONENT_STATUS_SYSTEM_TICK)
                DeviceComponent::components[i]->periodicCallback();

            i++;
        }
    }

    if(evt.value == DEVICE_SCHEDULER_EVT_IDLE)
    {
        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(DeviceComponent::components[i] && DeviceComponent::components[i]->status & DEVICE_COMPONENT_STATUS_IDLE_TICK)
                DeviceComponent::components[i]->idleCallback();

            i++;
        }
    }
}

/**
  * Adds the current DeviceComponent instance to our array of components.
  */
void DeviceComponent::addComponent()
{
    uint8_t i = 0;

    // iterate through our list until an empty space is found.
    while(i < DEVICE_COMPONENT_COUNT)
    {
        if(components[i] == NULL)
        {
            components[i] = this;
            break;
        }

        i++;
    }

    if(!(configuration & DEVICE_COMPONENT_LISTENERS_CONFIGURED) && EventModel::defaultEventBus)
    {
        int ret = system_timer_event_every_us(SCHEDULER_TICK_PERIOD_US, DEVICE_ID_COMPONENT, DEVICE_COMPONENT_EVT_SYSTEM_TICK);

        if(ret == DEVICE_OK)
        {
            EventModel::defaultEventBus->listen(DEVICE_ID_COMPONENT, DEVICE_COMPONENT_EVT_SYSTEM_TICK, component_callback, MESSAGE_BUS_LISTENER_IMMEDIATE);
            EventModel::defaultEventBus->listen(DEVICE_ID_SCHEDULER, DEVICE_SCHEDULER_EVT_IDLE, component_callback, MESSAGE_BUS_LISTENER_IMMEDIATE);

            DeviceComponent::configuration |= DEVICE_COMPONENT_LISTENERS_CONFIGURED;
        }
    }
}

/**
  * Removes the current DeviceComponent instance from our array of components.
  */
void DeviceComponent::removeComponent()
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
