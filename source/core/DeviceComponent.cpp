#include "DeviceComponent.h"
#include "DeviceSystemTimer.h"
#include "EventModel.h"

DeviceComponent* DeviceComponent::components[DEVICE_COMPONENT_COUNT];

uint8_t DeviceComponent::configuration;

/**
  * The periodic callback for all components.
  */
void component_callback(DeviceEvent)
{
    uint8_t i = 0;

    while(i < DEVICE_COMPONENT_COUNT)
    {
        if(DeviceComponent::components[i] && DeviceComponent::components[i]->status & DEVICE_COMPONENT_STATUS_SYSTEM_TICK)
            DeviceComponent::components[i]->periodicCallback();

        i++;
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

    if(!(configuration & DEVICE_COMPONENT_LISTENER_CONFIGURED) && EventModel::defaultEventBus != NULL)
    {
        EventModel::defaultEventBus->listen(DEVICE_ID_COMPONENT, DEVICE_COMPONENT_EVT_TICK, component_callback, MESSAGE_BUS_LISTENER_IMMEDIATE);
        system_timer_event_every_us(DEVICE_ID_COMPONENT, DEVICE_COMPONENT_EVT_TICK, SCHEDULER_TICK_PERIOD_MS * 1000);

        DeviceComponent::configuration |= DEVICE_COMPONENT_LISTENER_CONFIGURED;
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
