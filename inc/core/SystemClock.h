#ifndef CODAL_SYSTEM_CLOCK_H
#define CODAL_SYSTEM_CLOCK_H

#include <stdint.h>

#include "DeviceComponent.h"
#include "list.h"
#include "ErrorNo.h"
#include "CodalDevice.h"

#define SYSTEM_CLOCK_INIT 0x01

extern CodalDevice& device;

struct ClockEvent
{
    uint16_t id;
    uint16_t value;
    uint64_t period;
    uint64_t timestamp;
    struct list_head list;

    static void removeFromList(uint16_t id, uint16_t value, list_head* head)
    {
        ClockEvent* tmp = NULL;
        struct list_head *iter, *q = NULL;

        list_for_each_safe(iter, q, head)
        {
            tmp = list_entry(iter, ClockEvent, list);

            if(tmp->id == id && tmp->value == value)
            {
                list_del(iter);
                delete tmp;
            }
        }
    }

    static void addToList(ClockEvent* evt, list_head* head)
    {
        if(list_empty(head))
            list_add(&evt->list, head);
        else
        {
            ClockEvent* tmp = NULL;
            struct list_head *iter, *q = NULL;

            // check for duplicates, and remove if there are any.
            removeFromList(evt->id, evt->value, head);

            // add the new ClockEvent.
            list_for_each_safe(iter, q, head)
            {
                if(iter == head)
                    continue;

                tmp = list_entry(iter, ClockEvent, list);

                if(tmp->timestamp >= evt->timestamp)
                {
                    list_add(&evt->list, iter->prev);
                    return;
                }

                if(iter->next == head)
                {
                    list_add(&evt->list, iter);
                    return;
                }
            }
        }
    }

    ClockEvent()
    {
        this->timestamp = 0;
        this->value = 0;
        this->id = 0;
        this->period = 0;
    }

    ClockEvent(uint64_t timestamp, uint64_t period, uint16_t id, uint16_t value, list_head* head, bool repeating = false)
    {
        this->timestamp = timestamp;
        this->id = id;
        this->value = value;

        this->period = repeating ? period : 0;

        device.disableInterrupts();
        addToList(this, head);
        device.enableInterrupts();
    };
};

struct ClockPrescalerConfig
{
    uint16_t prescaleValue;
    uint16_t register_config;
};

class SystemClock : protected DeviceComponent
{

public:

    /**
      * Constructor for a generic system clock interface.
      */
    SystemClock() {};

    /**
      * Initialises and starts this SystemClock instance
      */
    virtual int init() { return DEVICE_NOT_SUPPORTED; };

    /**
      * Retrieves the device component id for this SystemClock instance.
      */
    virtual int getId() { return DEVICE_NOT_SUPPORTED; };

    /**
      * Sets the current time tracked by this SystemClock instance
      * in milliseconds
      *
      * @param timestamp the new time for this SystemClock instance in milliseconds
      */
    virtual int setTime(uint64_t timestamp) { (void)timestamp; return DEVICE_NOT_SUPPORTED; };

    /**
      * Sets the current time tracked by this SystemClock instance
      * in microseconds
      *
      * @param timestamp the new time for this SystemClock instance in microseconds
      */
    virtual int setTimeUs(uint64_t timestamp){ (void)timestamp; return DEVICE_NOT_SUPPORTED;};

    /**
      * Retrieves the current time tracked by this SystemClock instance
      * in milliseconds
      *
      * @return the timestamp in milliseconds
      */
    virtual uint64_t getTime() { return 0; };

    /**
      * Retrieves the current time tracked by this SystemClock instance
      * in microseconds
      *
      * @return the timestamp in microseconds
      */
    virtual uint64_t getTimeUs() { return 0; };

    /**
      * Configures this SystemClock instance to fire an event after period
      * milliseconds.
      *
      * @param period the period to wait until an event is triggered, in milliseconds.
      *
      * @param id the ID to be used in event generation.
      *
      * @param value the value to place into the Events' value field.
      */
    virtual int eventAfter(uint64_t period, uint16_t id, uint16_t value)
    {
        (void) period;
        (void) id;
        (void) value;
        return DEVICE_NOT_SUPPORTED;
    };

    /**
      * Configures this SystemClock instance to fire an event after period
      * microseconds.
      *
      * @param period the period to wait until an event is triggered, in microseconds.
      *
      * @param id the ID to be used in event generation.
      *
      * @param value the value to place into the Events' value field.
      */
    virtual int eventAfterUs(uint64_t period, uint16_t id, uint16_t value)
    {
        (void) period;
        (void) id;
        (void) value;
        return DEVICE_NOT_SUPPORTED;
    };

    /**
      * Configures this SystemClock instance to fire an event every period
      * milliseconds.
      *
      * @param period the period to wait until an event is triggered, in milliseconds.
      *
      * @param id the ID to be used in event generation.
      *
      * @param value the value to place into the Events' value field.
      */
    virtual int eventEvery(uint64_t period, uint16_t id, uint16_t value)
    {
        (void) period;
        (void) id;
        (void) value;
        return DEVICE_NOT_SUPPORTED;
    };

    /**
      * Configures this SystemClock instance to fire an event every period
      * microseconds.
      *
      * @param period the period to wait until an event is triggered, in microseconds.
      *
      * @param id the ID to be used in event generation.
      *
      * @param value the value to place into the Events' value field.
      */
    virtual int eventEveryUs(uint64_t period, uint16_t id, uint16_t value)
    {
        (void) period;
        (void) id;
        (void) value;
        return DEVICE_NOT_SUPPORTED;
    };

    /**
      * Cancels any events matching the given id and value.
      *
      * @param id the ID that was given upon a previous call to eventEvery / eventAfter
      *
      * @param value the value that was given upon a previous call to eventEvery / eventAfter
      */
    virtual int cancel(uint16_t id, uint16_t value)
    {
        (void) id;
        (void) value;
        return DEVICE_NOT_SUPPORTED;
    };

    /**
      * Start this SystemClock instance.
      *
      * @param precisionUs The precisions that the timer should use.
      */
    virtual int start(uint64_t precisionUs) { (void)precisionUs; return DEVICE_NOT_SUPPORTED; };

    /**
      * Stop this SystemClock instance
      */
    virtual int stop() { return DEVICE_NOT_SUPPORTED; };

    /**
      * Destructor for this SystemClock instance
      */
    virtual ~SystemClock() { stop(); };
};

#endif
