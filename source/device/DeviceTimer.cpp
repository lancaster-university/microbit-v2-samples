#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"

static uint64_t current_time_us = 0;
static uint8_t compare_event = 0;
static uint16_t timer_id = 0;
static uint32_t overflow_period = 0;


static Ticker* ticker = NULL;
static Timer* timer = NULL;

LIST_HEAD(event_list);

void consume_events();

void set_interrupt(uint32_t time_us)
{
    if(time_us > overflow_period)
        return;

    compare_event = 0;
    ticker->detach();
    ticker->attach_us(consume_events,time_us);

    return;
}

void consume_events()
{
    if(list_empty(&event_list))
        return;

    ClockEvent* tmp = NULL;
    struct list_head *iter, *q = NULL;

    uint32_t period_us = timer->read_us();
    timer->reset();

    current_time_us += period_us;

    uint8_t interrupt_set = 0;
    uint8_t first = 1;

    list_for_each_safe(iter, q, &event_list)
    {
        tmp = list_entry(iter, ClockEvent, list);

        // if we have received a compare event, we know we will be ripping off the top!
        if(first && compare_event)
        {
            first = 0;
            compare_event = 0;

            // fire our event and process the next event
            DeviceEvent(timer_id, tmp->value);

            // remove from the event list
            list_del(iter);

            // if this event is repeating, reset our timestamp
            if(tmp->period == 0)
            {
                delete tmp;
                continue;
            }
            else
            {
                // update our count, and readd to our event list
                tmp->countUs = tmp->period;
                tmp->addToList(&event_list);
            }
        }
        else
            tmp->countUs -= period_us;

        if(!interrupt_set && tmp->countUs < overflow_period)
        {
            set_interrupt(tmp->countUs);
            interrupt_set = 1;
        }
    }
}

/**
  * Constructor for an instance of Timer1.
  *
  * @param id The id to use for the message bus when transmitting events.
  */
DeviceTimer::DeviceTimer(uint16_t id)
{
    timer = NULL;
    ticker = NULL;
    this->id = timer_id = id;
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

    start();

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
    current_time_us += timer->read_us();
    timer->reset();
    return current_time_us;
}

/**
  * Configures this DeviceTimer instance to fire an event after period
  * milliseconds.
  *
  * @param period the period to wait until an event is triggered, in milliseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventAfter(uint64_t interval, uint16_t value)
{
    return eventAfterUs(interval * 1000, value);
}

/**
  * Configures this DeviceTimer instance to fire an event after period
  * microseconds.
  *
  * @param period the period to wait until an event is triggered, in microseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventAfterUs(uint64_t interval, uint16_t value)
{
    if(new ClockEvent(interval, value, &event_list))
        return DEVICE_OK;

    return DEVICE_NO_RESOURCES;
}

/**
  * Configures this DeviceTimer instance to fire an event every period
  * milliseconds.
  *
  * @param period the period to wait until an event is triggered, in milliseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventEvery(uint64_t period, uint16_t value)
{
    return eventEveryUs(period * 1000, value);
}

/**
  * Configures this DeviceTimer instance to fire an event every period
  * microseconds.
  *
  * @param period the period to wait until an event is triggered, in microseconds.
  *
  * @param value the value to place into the Events' value field.
  */
int DeviceTimer::eventEveryUs(uint64_t period, uint16_t value)
{
    ClockEvent* clk = new ClockEvent(period, value, &event_list, true);

    if(!clk)
        return DEVICE_NO_RESOURCES;

    if(event_list.next == &clk->list && period < overflow_period)
    {
        __disable_irq();
        set_interrupt(period);
        __enable_irq();
    }

    return DEVICE_OK;
}

/**
  * Start this DeviceTimer instance.
  *
  * @param precisionUs The precisions that the timer should use. Defaults to
  *        TIMER_ONE_DEFAULT_PRECISION_US (1 us)
  */
int DeviceTimer::start(uint64_t precisionUs)
{
    if(!ticker)
        ticker = new Ticker();

    if(!timer)
        timer = new Timer();

    timer->start();
    ticker->attach_us(consume_events, precisionUs);

    return DEVICE_OK;
}

/**
  * Stop this DeviceTimer instance
  */
int DeviceTimer::stop()
{
    if(!status & SYSTEM_CLOCK_INIT)
        return DEVICE_NOT_SUPPORTED;

    timer->stop();
    ticker->detach();

    return DEVICE_OK;
}

DeviceTimer::~DeviceTimer()
{
    stop();

    if(ticker)
        free(ticker);

    if(timer)
        free(timer);
}
