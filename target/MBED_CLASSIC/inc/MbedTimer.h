#ifndef CODAL_MBED_TIMER_H
#define CODAL_MBED_TIMER_H

#include "Timer.h"
#include "mbed.h"

#define TIMER_ONE_DEFAULT_PRECISION_US    1
#define TIMER_ONE_PRESCALER_OPTIONS       5

namespace mb=mbed;
namespace codal
{
    namespace mbed
    {
        class Timer : public codal::Timer
        {
            mb::Timer timer;
            mb::Timeout timeout;
            mb::Timeout overflowTimeout;

            void processEvents();

            void timerOverflow();

            int configureEvent(uint64_t period, uint16_t id, uint16_t value, bool repeating);

        public:

            /**
              * Constructor for an instance of Timer1.
              *
              * @param id The id to use for the message bus when transmitting events.
              */
            Timer(uint16_t id = DEVICE_ID_SYSTEM_TIMER);

            /**
              * Returns the id for this timer instance
              */
            int getId();

            /**
              * Initialises and starts this Timer1 instance
              */
            int init();

            /**
              * Sets the current time tracked by this Timer1 instance
              * in milliseconds
              *
              * @param timestamp the new time for this Timer1 instance in milliseconds
              */
            int setTime(uint64_t timestamp);

            /**
              * Sets the current time tracked by this Timer1 instance
              * in microseconds
              *
              * @param timestamp the new time for this Timer1 instance in microseconds
              */
            int setTimeUs(uint64_t timestamp);

            /**
              * Retrieves the current time tracked by this Timer1 instance
              * in milliseconds
              *
              * @return the timestamp in milliseconds
              */
            uint64_t getTime();

            /**
              * Retrieves the current time tracked by this Timer1 instance
              * in microseconds
              *
              * @return the timestamp in microseconds
              */
            uint64_t getTimeUs();

            /**
              * Configures this Timer1 instance to fire an event after period
              * milliseconds.
              *
              * @param period the period to wait until an event is triggered, in milliseconds.
              *
              * @param value the value to place into the Events' value field.
              */
            int eventAfter(uint64_t interval, uint16_t id, uint16_t value);

            /**
              * Configures this Timer1 instance to fire an event after period
              * microseconds.
              *
              * @param period the period to wait until an event is triggered, in microseconds.
              *
              * @param value the value to place into the Events' value field.
              */
            int eventAfterUs(uint64_t interval, uint16_t id, uint16_t value);

            /**
              * Configures this Timer1 instance to fire an event every period
              * milliseconds.
              *
              * @param period the period to wait until an event is triggered, in milliseconds.
              *
              * @param value the value to place into the Events' value field.
              */
            int eventEvery(uint64_t period, uint16_t id, uint16_t value);

            /**
              * Configures this Timer1 instance to fire an event every period
              * microseconds.
              *
              * @param period the period to wait until an event is triggered, in microseconds.
              *
              * @param value the value to place into the Events' value field.
              */
            int eventEveryUs(uint64_t period, uint16_t id, uint16_t value);

            /**
              * Cancels any events matching the given id and value.
              *
              * @param id the ID that was given upon a previous call to eventEvery / eventAfter
              *
              * @param value the value that was given upon a previous call to eventEvery / eventAfter
              */
            int cancel(uint16_t id, uint16_t value);
        };
    }
}

#endif
