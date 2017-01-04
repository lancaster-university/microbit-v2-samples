#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"

static uint64_t current_time_us = 0;
static uint32_t overflow_period_us = 0;

static ClockEvent event_list;

void DeviceTimer::processEvents()
{
    if(list_empty(&event_list.list))
        return;

    ClockEvent* tmp = NULL;

    struct list_head* iter, *q;

    list_for_each_safe(iter, q, &event_list.list)
    {
       tmp = list_entry(iter, ClockEvent, list);

       // fire our event and process the next event
       DeviceEvent(tmp->id, tmp->value);

       // remove from the event list
       list_del(iter);

       // if this event is non-repeating, delete
       if(tmp->period == 0)
          delete tmp;
       else
       {
          // update our count, and readd to our event list
          tmp->timestamp = getTimeUs() + tmp->period;
          ClockEvent::addToList(tmp, &event_list.list);
       }

       if(tmp->timestamp > getTimeUs())
          break;
    }

    uint64_t usRemaining = tmp->timestamp - getTimeUs();

    if(usRemaining < overflow_period_us)
        timeout.attach_us(this, &DeviceTimer::processEvents, usRemaining);
}

void DeviceTimer::timerOverflow()
{
    timer.reset();
    overflowTimeout.attach_us(this, &DeviceTimer::timerOverflow, overflow_period_us);

    current_time_us += overflow_period_us;

    processEvents();
}

/**
  * Constructor for an instance of Timer1.
  *
  * @param id The id to use for the message bus when transmitting events.
  */
DeviceTimer::DeviceTimer(uint16_t id) : timer(), timeout(), overflowTimeout()
{
    if(system_timer_get_instance() == NULL)
        system_timer_set_instance(this);

    INIT_LIST_HEAD(&event_list.list);

    overflow_period_us = 60000000;

    this->id = id;
}

/**
  * Returns the id for this timer instance
  */
int DeviceTimer::getId()
{
    return this->id;
}

/**
  * Initialises and starts this DeviceTimer instance
  */
int DeviceTimer::init()
{
    if(status & SYSTEM_CLOCK_INIT)
        return DEVICE_OK;

    timer.start();

    overflowTimeout.attach_us(this, &DeviceTimer::timerOverflow, overflow_period_us);

    status |= SYSTEM_CLOCK_INIT;

    return DEVICE_OK;
}

/**
  * Sets the current time tracked by this DeviceTimer instance
  * in milliseconds
  *
  * @param timestamp the new time for this DeviceTimer instance in milliseconds
  */
int DeviceTimer::setTime(uint64_t timestamp)
{
    return setTimeUs(timestamp * 1000);
}

/**
  * Sets the current time tracked by this DeviceTimer instance
  * in microseconds
  *
  * @param timestamp the new time for this DeviceTimer instance in microseconds
  */
int DeviceTimer::setTimeUs(uint64_t timestamp)
{
    current_time_us = timestamp;
    return DEVICE_OK;
}

/**
  * Retrieves the current time tracked by this DeviceTimer instance
  * in milliseconds
  *
  * @return the timestamp in milliseconds
  */
uint64_t DeviceTimer::getTime()
{
    return getTimeUs() / 1000;
}

/**
  * Retrieves the current time tracked by this DeviceTimer instance
  * in microseconds
  *
  * @return the timestamp in microseconds
  */
uint64_t DeviceTimer::getTimeUs()
{
    return current_time_us + timer.read_us();
}

int DeviceTimer::configureEvent(uint64_t period, uint16_t id, uint16_t value, bool repeating)
{
    ClockEvent* clk = new ClockEvent(getTimeUs() + period, period, id, value, &event_list.list, repeating);

    if(!clk)
        return DEVICE_NO_RESOURCES;

    if(event_list.list.next == &clk->list && period < overflow_period_us)
        timeout.attach_us(this, &DeviceTimer::processEvents, period);

    return DEVICE_OK;
}

/**
  * Configures this DeviceTimer instance to fire an event after period
  * milliseconds.
  *
  * @param period the period to wait until an event is triggered, in milliseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventAfter(uint64_t interval, uint16_t id, uint16_t value)
{
    return eventAfterUs(interval * 1000, id, value);
}

/**
  * Configures this DeviceTimer instance to fire an event after period
  * microseconds.
  *
  * @param period the period to wait until an event is triggered, in microseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventAfterUs(uint64_t interval, uint16_t id, uint16_t value)
{
    return configureEvent(interval, id, value, false);
}

/**
  * Configures this DeviceTimer instance to fire an event every period
  * milliseconds.
  *
  * @param period the period to wait until an event is triggered, in milliseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventEvery(uint64_t period, uint16_t id, uint16_t value)
{
    return eventEveryUs(period * 1000, id, value);
}

/**
  * Configures this DeviceTimer instance to fire an event every period
  * microseconds.
  *
  * @param period the period to wait until an event is triggered, in microseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventEveryUs(uint64_t period, uint16_t id, uint16_t value)
{
    return configureEvent(period, id, value, true);
}

/**
  * Cancels any events matching the given id and value.
  *
  * @param id the ID that was given upon a previous call to eventEvery / eventAfter
  *
  * @param value the value that was given upon a previous call to eventEvery / eventAfter
  */
int DeviceTimer::cancel(uint16_t id, uint16_t value)
{
    __disable_irq();
    ClockEvent::removeFromList(id, value, &event_list.list);
    __enable_irq();
    return DEVICE_OK;
}
