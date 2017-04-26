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

/**
  * Class definition for the DeviceMessageBus.
  *
  * The DeviceMessageBus is the common mechanism to deliver asynchronous events on the
  * Device platform. It serves a number of purposes:
  *
  * 1) It provides an eventing abstraction that is independent of the underlying substrate.
  *
  * 2) It provides a mechanism to decouple user code from trusted system code
  *    i.e. the basis of a message passing nano kernel.
  *
  * 3) It allows a common high level eventing abstraction across a range of hardware types.e.g. buttons, BLE...
  *
  * 4) It provides a mechanim for extensibility - new devices added via I/O pins can have OO based
  *    drivers and communicate via the message bus with minima impact on user level languages.
  *
  * 5) It allows for the possiblility of event / data aggregation, which in turn can save energy.
  *
  * It has the following design principles:
  *
  * 1) Maintain a low RAM footprint where possible
  *
  * 2) Make few assumptions about the underlying platform, but allow optimizations where possible.
  */
#include "DeviceConfig.h"
#include "DeviceMessageBus.h"
#include "DeviceFiber.h"
#include "ErrorNo.h"

using namespace codal;

/**
  * Default constructor.
  *
  * Adds itself as a fiber component, and also configures itself to be the
  * default EventModel if defaultEventBus is NULL.
  */
DeviceMessageBus::DeviceMessageBus()
{
    this->listeners = NULL;
    this->evt_queue_head = NULL;
    this->evt_queue_tail = NULL;
    this->queueLength = 0;

    // ANY listeners for scheduler events MUST be immediate, or else they will not be registered.
    listen(DEVICE_ID_SCHEDULER, DEVICE_SCHEDULER_EVT_IDLE, this, &DeviceMessageBus::idle, MESSAGE_BUS_LISTENER_IMMEDIATE);

    if(EventModel::defaultEventBus == NULL)
        EventModel::defaultEventBus = this;
}

/**
  * Invokes a callback on a given DeviceListener
  *
  * Internal wrapper function, used to enable
  * parameterised callbacks through the fiber scheduler.
  */
void async_callback(void *param)
{
    DeviceListener *listener = (DeviceListener *)param;

    // OK, now we need to decide how to behave depending on our configuration.
    // If this a fiber f already active within this listener then check our
    // configuration to determine the correct course of action.
    //

    if (listener->flags & MESSAGE_BUS_LISTENER_BUSY)
    {
        // Drop this event, if that's how we've been configured.
        if (listener->flags & MESSAGE_BUS_LISTENER_DROP_IF_BUSY)
            return;

        // Queue this event up for later, if that's how we've been configured.
        if (listener->flags & MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY)
        {
            listener->queue(listener->evt);
            return;
        }
    }

    // Determine the calling convention for the callback, and invoke...
    // C++ is really bad at this! Especially as the ARM compiler is yet to support C++ 11 :-/

    // Record that we have a fiber going into this listener...
    listener->flags |= MESSAGE_BUS_LISTENER_BUSY;

    while (1)
    {
        // Firstly, check for a method callback into an object.
        if (listener->flags & MESSAGE_BUS_LISTENER_METHOD)
            listener->cb_method->fire(listener->evt);

        // Now a parameterised C function
        else if (listener->flags & MESSAGE_BUS_LISTENER_PARAMETERISED)
            listener->cb_param(listener->evt, listener->cb_arg);

        // We must have a plain C function
        else
            listener->cb(listener->evt);


        // If there are more events to process, dequeue the next one and process it.
        if ((listener->flags & MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY) && listener->evt_queue)
        {
            DeviceEventQueueItem *item = listener->evt_queue;

            listener->evt = item->evt;
            listener->evt_queue = listener->evt_queue->next;
            delete item;

            // We spin the scheduler here, to preven any particular event handler from continuously holding onto resources.
            schedule();
        }
        else
            break;
    }

    // The fiber of exiting... clear our state.
    listener->flags &= ~MESSAGE_BUS_LISTENER_BUSY;
}

/**
  * Queue the given event for processing at a later time.
  * Add the given event at the tail of our queue.
  *
  * @param The event to queue.
  */
void DeviceMessageBus::queueEvent(DeviceEvent &evt)
{
    int processingComplete;

    DeviceEventQueueItem *prev = evt_queue_tail;

    // Now process all handler regsitered as URGENT.
    // These pre-empt the queue, and are useful for fast, high priority services.
    processingComplete = this->process(evt, true);

    // If we've already processed all event handlers, we're all done.
    // No need to queue the event.
    if (processingComplete)
        return;

    // If we need to queue, but there is no space, then there's nothg we can do.
    if (queueLength >= MESSAGE_BUS_LISTENER_MAX_QUEUE_DEPTH)
        return;

    // Otherwise, we need to queue this event for later processing...
    // We queue this event at the tail of the queue at the point where we entered queueEvent()
    // This is important as the processing above *may* have generated further events, and
    // we want to maintain ordering of events.
    DeviceEventQueueItem *item = new DeviceEventQueueItem(evt);

    // The queue was empty when we entered this function, so queue our event at the start of the queue.
    device.disableInterrupts();

    if (prev == NULL)
    {
        item->next = evt_queue_head;
        evt_queue_head = item;
    }
    else
    {
        item->next = prev->next;
        prev->next = item;
    }

    if (item->next == NULL)
        evt_queue_tail = item;

    queueLength++;

    device.enableInterrupts();
}

/**
  * Extract the next event from the front of the event queue (if present).
  *
  * @return a pointer to the DeviceEventQueueItem that is at the head of the list.
  */
DeviceEventQueueItem* DeviceMessageBus::dequeueEvent()
{
    DeviceEventQueueItem *item = NULL;

    device.disableInterrupts();

    if (evt_queue_head != NULL)
    {
        item = evt_queue_head;
        evt_queue_head = item->next;

        if (evt_queue_head == NULL)
            evt_queue_tail = NULL;

        queueLength--;
    }

    device.enableInterrupts();

    return item;
}

/**
  * Cleanup any DeviceListeners marked for deletion from the list.
  *
  * @return The number of listeners removed from the list.
  */
int DeviceMessageBus::deleteMarkedListeners()
{
    DeviceListener *l, *p;
    int removed = 0;

    l = listeners;
    p = NULL;

    // Walk this list of event handlers. Delete any that match the given listener.
    while (l != NULL)
    {
        if ((l->flags & MESSAGE_BUS_LISTENER_DELETING) && !(l->flags & MESSAGE_BUS_LISTENER_BUSY))
        {
            if (p == NULL)
                listeners = l->next;
            else
                p->next = l->next;

            // delete the listener.
            DeviceListener *t = l;
            l = l->next;

            delete t;
            removed++;

            continue;
        }

        p = l;
        l = l->next;
    }

    return removed;
}

/**
  * Periodic callback from Device.
  *
  * Process at least one event from the event queue, if it is not empty.
  * We then continue processing events until something appears on the runqueue.
  */
void DeviceMessageBus::idle(DeviceEvent)
{
    // Clear out any listeners marked for deletion
    this->deleteMarkedListeners();

    DeviceEventQueueItem *item = this->dequeueEvent();

    // Whilst there are events to process and we have no useful other work to do, pull them off the queue and process them.
    while (item)
    {
        // send the event to all standard event listeners.
        this->process(item->evt);

        // Free the queue item.
        delete item;

        // If we have created some useful work to do, we stop processing.
        // This helps to minimise the number of blocked fibers we create at any point in time, therefore
        // also reducing the RAM footprint.
        if(!scheduler_runqueue_empty())
            break;

        // Pull the next event to process, if there is one.
        item = this->dequeueEvent();
    }
}

/**
  * Queues the given event to be sent to all registered recipients.
  *
  * @param evt The event to send.
  *
  * @code
  * DeviceMessageBus bus;
  *
  * // Creates and sends the DeviceEvent using bus.
  * DeviceEvent evt(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK);
  *
  * // Creates the DeviceEvent, but delays the sending of that event.
  * DeviceEvent evt1(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK, CREATE_ONLY);
  *
  * bus.send(evt1);
  *
  * // This has the same effect!
  * evt1.fire()
  * @endcode
  */
int DeviceMessageBus::send(DeviceEvent evt)
{
    // We simply queue processing of the event until we're scheduled in normal thread context.
    // We do this to avoid the possibility of executing event handler code in IRQ context, which may bring
    // hidden race conditions to kids code. Queuing all events ensures causal ordering (total ordering in fact).
    this->queueEvent(evt);
    return DEVICE_OK;
}

/**
  * Internal function, used to deliver the given event to all relevant recipients.
  * Normally, this is called once an event has been removed from the event queue.
  *
  * @param evt The event to send.
  *
  * @param urgent The type of listeners to process (optional). If set to true, only listeners defined as urgent and non-blocking will be processed
  *               otherwise, all other (standard) listeners will be processed. Defaults to false.
  *
  * @return 1 if all matching listeners were processed, 0 if further processing is required.
  *
  * @note It is recommended that all external code uses the send() function instead of this function,
  *       or the constructors provided by DeviceEvent.
  */
int DeviceMessageBus::process(DeviceEvent &evt, bool urgent)
{
    DeviceListener *l;
    int complete = 1;
    bool listenerUrgent;

    l = listeners;

    while (l != NULL)
    {
        if((l->id == evt.source || l->id == DEVICE_ID_ANY) && (l->value == evt.value || l->value == DEVICE_EVT_ANY))
        {
            // If we're running under the fiber scheduler, then derive the THREADING_MODE for the callback based on the
            // metadata in the listener itself.
            if (fiber_scheduler_running())
                listenerUrgent = (l->flags & MESSAGE_BUS_LISTENER_IMMEDIATE) == MESSAGE_BUS_LISTENER_IMMEDIATE;
            else
                listenerUrgent = true;

            // If we should process this event hander in this pass, then activate the listener.
            if(listenerUrgent == urgent && !(l->flags & MESSAGE_BUS_LISTENER_DELETING))
            {
                l->evt = evt;

                // OK, if this handler has regisitered itself as non-blocking, we just execute it directly...
                // This is normally only done for trusted system components.
                // Otherwise, we invoke it in a 'fork on block' context, that will automatically create a fiber
                // should the event handler attempt a blocking operation, but doesn't have the overhead
                // of creating a fiber needlessly. (cool huh?)
                if (l->flags & MESSAGE_BUS_LISTENER_NONBLOCKING || !fiber_scheduler_running())
                    async_callback(l);
                else
                    invoke(async_callback, l);
            }
            else
                complete = 0;
        }

        l = l->next;
    }

    //Serial.println("EXIT");
    //while (!(UCSR0A & _BV(TXC0)));

    return complete;
}

/**
  * Add the given DeviceListener to the list of event handlers, unconditionally.
  *
  * @param listener The DeviceListener to add.
  *
  * @return DEVICE_OK if the listener is valid, DEVICE_INVALID_PARAMETER otherwise.
  */
int DeviceMessageBus::add(DeviceListener *newListener)
{
    DeviceListener *l, *p;
    int methodCallback;

    //handler can't be NULL!
    if (newListener == NULL)
        return DEVICE_INVALID_PARAMETER;

    l = listeners;

    // Firstly, we treat a listener as an idempotent operation. Ensure we don't already have this handler
    // registered in a that will already capture these events. If we do, silently ignore.

    // We always check the ID, VALUE and CB_METHOD fields.
    // If we have a callback to a method, check the cb_method class. Otherwise, the cb function point is sufficient.
    while (l != NULL)
    {
        methodCallback = (newListener->flags & MESSAGE_BUS_LISTENER_METHOD) && (l->flags & MESSAGE_BUS_LISTENER_METHOD);

        if (l->id == newListener->id && l->value == newListener->value && (methodCallback ? *l->cb_method == *newListener->cb_method : l->cb == newListener->cb))
        {
            // We have a perfect match for this event listener already registered.
            // If it's marked for deletion, we simply resurrect the listener, and we're done.
            // Either way, we return an error code, as the *new* listener should be released...
            if(l->flags & MESSAGE_BUS_LISTENER_DELETING)
                l->flags &= ~MESSAGE_BUS_LISTENER_DELETING;

            return DEVICE_NOT_SUPPORTED;
        }

        l = l->next;
    }

    // We have a valid, new event handler. Add it to the list.
    // if listeners is null - we can automatically add this listener to the list at the beginning...
    if (listeners == NULL)
    {
        listeners = newListener;
        DeviceEvent(DEVICE_ID_MESSAGE_BUS_LISTENER, newListener->id);

        return DEVICE_OK;
    }

    // We maintain an ordered list of listeners.
    // The chain is held stictly in increasing order of ID (first level), then value code (second level).
    // Find the correct point in the chain for this event.
    // Adding a listener is a rare occurance, so we just walk the list...

    p = listeners;
    l = listeners;

    while (l != NULL && l->id < newListener->id)
    {
        p = l;
        l = l->next;
    }

    while (l != NULL && l->id == newListener->id && l->value < newListener->value)
    {
        p = l;
        l = l->next;
    }

    //add at front of list
    if (p == listeners && (newListener->id < p->id || (p->id == newListener->id && p->value > newListener->value)))
    {
        newListener->next = p;

        //this new listener is now the front!
        listeners = newListener;
    }

    //add after p
    else
    {
        newListener->next = p->next;
        p->next = newListener;
    }

    DeviceEvent(DEVICE_ID_MESSAGE_BUS_LISTENER, newListener->id);
    return DEVICE_OK;
}

/**
  * Remove the given DeviceListener from the list of event handlers.
  *
  * @param listener The DeviceListener to remove.
  *
  * @return DEVICE_OK if the listener is valid, DEVICE_INVALID_PARAMETER otherwise.
  */
int DeviceMessageBus::remove(DeviceListener *listener)
{
    DeviceListener *l;
    int removed = 0;

    //handler can't be NULL!
    if (listener == NULL)
        return DEVICE_INVALID_PARAMETER;

    l = listeners;

    // Walk this list of event handlers. Delete any that match the given listener.
    while (l != NULL)
    {
        if ((listener->flags & MESSAGE_BUS_LISTENER_METHOD) == (l->flags & MESSAGE_BUS_LISTENER_METHOD))
        {
            if(((listener->flags & MESSAGE_BUS_LISTENER_METHOD) && (*l->cb_method == *listener->cb_method)) ||
              ((!(listener->flags & MESSAGE_BUS_LISTENER_METHOD) && l->cb == listener->cb)))
            {
                if ((listener->id == DEVICE_ID_ANY || listener->id == l->id) && (listener->value == DEVICE_EVT_ANY || listener->value == l->value))
                {
                    // Found a match. mark this to be removed from the list.
                    l->flags |= MESSAGE_BUS_LISTENER_DELETING;
                    removed++;
                }
            }
        }

        l = l->next;
    }

    if (removed > 0)
        return DEVICE_OK;
    else
        return DEVICE_INVALID_PARAMETER;
}

/**
  * Returns the microBitListener with the given position in our list.
  *
  * @param n The position in the list to return.
  *
  * @return the DeviceListener at postion n in the list, or NULL if the position is invalid.
  */
DeviceListener* DeviceMessageBus::elementAt(int n)
{
    DeviceListener *l = listeners;

    while (n > 0)
    {
        if (l == NULL)
            return NULL;

        n--;
        l = l->next;
    }

    return l;
}

/**
  * Destructor for DeviceMessageBus, where we deregister this instance from the array of fiber components.
  */
DeviceMessageBus::~DeviceMessageBus()
{
    ignore(DEVICE_ID_SCHEDULER, DEVICE_EVT_ANY, this, &DeviceMessageBus::idle);
}
