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
  *	This structure defines a DeviceListener used to invoke functions, or member
  * functions if an instance of EventModel receives an event whose id and value
  * match this DeviceListener's id and value.
  */
#include "DeviceConfig.h"
#include "DeviceListener.h"

using namespace codal;

/**
  * Constructor.
  *
  * Create a new Message Bus Listener.
  *
  * @param id The ID of the component you want to listen to.
  *
  * @param value The event value you would like to listen to from that component
  *
  * @param handler A function pointer to call when the event is detected.
  *
  * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
  * to be tuned.
  */
DeviceListener::DeviceListener(uint16_t id, uint16_t value, void (*handler)(DeviceEvent), uint16_t flags)
{
	this->id = id;
	this->value = value;
	this->cb = handler;
	this->cb_arg = NULL;
    this->flags = flags;
	this->next = NULL;
    this->evt_queue = NULL;
}

/**
  * Constructor.
  *
  * Create a new Message Bus Listener, this constructor accepts an additional
  * parameter "arg", which is passed to the handler.
  *
  * @param id The ID of the component you want to listen to.
  *
  * @param value The event value you would like to listen to from that component
  *
  * @param handler A function pointer to call when the event is detected.
  *
  * @param arg A pointer to some data that will be given to the handler.
  *
  * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
  * to be tuned.
  */
DeviceListener::DeviceListener(uint16_t id, uint16_t value, void (*handler)(DeviceEvent, void *), void* arg, uint16_t flags)
{
	this->id = id;
	this->value = value;
	this->cb_param = handler;
	this->cb_arg = arg;
    this->flags = flags | MESSAGE_BUS_LISTENER_PARAMETERISED;
	this->next = NULL;
    this->evt_queue = NULL;
}

/**
  * Destructor. Ensures all resources used by this listener are freed.
  */
DeviceListener::~DeviceListener()
{
    if(this->flags & MESSAGE_BUS_LISTENER_METHOD)
        delete cb_method;
}

/**
  * Queues and event up to be processed.
  *
  * @param e The event to queue
  */
void DeviceListener::queue(DeviceEvent e)
{
    int queueDepth;

    DeviceEventQueueItem *p = evt_queue;

    if (evt_queue == NULL)
        evt_queue = new DeviceEventQueueItem(e);
    else
    {
        queueDepth = 1;

        while (p->next != NULL)
        {
            p = p->next;
            queueDepth++;
        }

        if (queueDepth < MESSAGE_BUS_LISTENER_MAX_QUEUE_DEPTH)
            p->next = new DeviceEventQueueItem(e);
    }
}
