#include "MbedTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "Timer.h"

namespace codal
{
    namespace mbed
    {
        static uint64_t current_time_us = 0;
        static uint32_t overflow_period_us = 0;

        static TimerEvent event_list;

        void Timer::processEvents()
        {
            START:

            if(list_empty(&event_list.list))
                return;

            TimerEvent* tmp = NULL;

            struct list_head* iter, *q;

            list_for_each_safe(iter, q, &event_list.list)
            {
               tmp = list_entry(iter, TimerEvent, list);

               if(tmp->timestamp > getTimeUs())
                  break;

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
                  tmp->timestamp += tmp->period;
                  TimerEvent::addToList(tmp, &event_list.list);
               }
            }

            // Take the new head of the list (which may have changed)
            // as this will be the evne that is due next.
            tmp = list_entry(event_list.list.next, TimerEvent, list);

            uint64_t now = getTimeUs();
            //uint64_t usRemaining = tmp->timestamp > now ? tmp->timestamp - now : 10;

            uint64_t usRemaining = tmp->timestamp - now;

            if (tmp->timestamp < now || usRemaining < 100)
                goto START;

            if(usRemaining < overflow_period_us)
                timeout.attach_us(this, &Timer::processEvents, usRemaining);
        }

        void Timer::timerOverflow()
        {
            timer.reset();
            overflowTimeout.attach_us(this, &Timer::timerOverflow, overflow_period_us);

            current_time_us += overflow_period_us;

            processEvents();
        }

        /**
          * Constructor for an instance of Timer1.
          *
          * @param id The id to use for the message bus when transmitting events.
          */
        Timer::Timer(uint16_t id) : timer(), timeout(), overflowTimeout()
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
        int Timer::getId()
        {
            return this->id;
        }

        /**
          * Initialises and starts this Timer instance
          */
        int Timer::init()
        {
            if(status & SYSTEM_CLOCK_INIT)
                return DEVICE_OK;

            timer.start();

            overflowTimeout.attach_us(this, &Timer::timerOverflow, overflow_period_us);

            status |= SYSTEM_CLOCK_INIT;

            return DEVICE_OK;
        }

        /**
          * Sets the current time tracked by this Timer instance
          * in milliseconds
          *
          * @param timestamp the new time for this Timer instance in milliseconds
          */
        int Timer::setTime(uint64_t timestamp)
        {
            return setTimeUs(timestamp * 1000);
        }

        /**
          * Sets the current time tracked by this Timer instance
          * in microseconds
          *
          * @param timestamp the new time for this Timer instance in microseconds
          */
        int Timer::setTimeUs(uint64_t timestamp)
        {
            current_time_us = timestamp;
            return DEVICE_OK;
        }

        /**
          * Retrieves the current time tracked by this Timer instance
          * in milliseconds
          *
          * @return the timestamp in milliseconds
          */
        uint64_t Timer::getTime()
        {
            return getTimeUs() / 1000;
        }

        /**
          * Retrieves the current time tracked by this Timer instance
          * in microseconds
          *
          * @return the timestamp in microseconds
          */
        uint64_t Timer::getTimeUs()
        {
            return current_time_us + timer.read_us();
        }

        int Timer::configureEvent(uint64_t period, uint16_t id, uint16_t value, bool repeating)
        {
            TimerEvent* clk = new TimerEvent(getTimeUs() + period, period, id, value, &event_list.list, repeating);

            if(!clk)
                return DEVICE_NO_RESOURCES;

            if(event_list.list.next == &clk->list && period < overflow_period_us)
                timeout.attach_us(this, &Timer::processEvents, period);

            return DEVICE_OK;
        }

        /**
          * Configures this Timer instance to fire an event after period
          * milliseconds.
          *
          * @param period the period to wait until an event is triggered, in milliseconds.
          *
          * @param value the value to place into the Events' value field.
          */
        int Timer::eventAfter(uint64_t interval, uint16_t id, uint16_t value)
        {
            return eventAfterUs(interval * 1000, id, value);
        }

        /**
          * Configures this Timer instance to fire an event after period
          * microseconds.
          *
          * @param period the period to wait until an event is triggered, in microseconds.
          *
          * @param value the value to place into the Events' value field.
          */
        int Timer::eventAfterUs(uint64_t interval, uint16_t id, uint16_t value)
        {
            return configureEvent(interval, id, value, false);
        }

        /**
          * Configures this Timer instance to fire an event every period
          * milliseconds.
          *
          * @param period the period to wait until an event is triggered, in milliseconds.
          *
          * @param value the value to place into the Events' value field.
          */
        int Timer::eventEvery(uint64_t period, uint16_t id, uint16_t value)
        {
            return eventEveryUs(period * 1000, id, value);
        }

        /**
          * Configures this Timer instance to fire an event every period
          * microseconds.
          *
          * @param period the period to wait until an event is triggered, in microseconds.
          *
          * @param value the value to place into the Events' value field.
          */
        int Timer::eventEveryUs(uint64_t period, uint16_t id, uint16_t value)
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
        int Timer::cancel(uint16_t id, uint16_t value)
        {
            __disable_irq();
            TimerEvent::removeFromList(id, value, &event_list.list);
            __enable_irq();
            return DEVICE_OK;
        }
    }
}
