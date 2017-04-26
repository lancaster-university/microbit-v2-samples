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
  * Functionality definitions for the Device Fiber scheduler.
  *
  * This lightweight, non-preemptive scheduler provides a simple threading mechanism for two main purposes:
  *
  * 1) To provide a clean abstraction for application languages to use when building async behaviour (callbacks).
  * 2) To provide ISR decoupling for EventModel events generated in an ISR context.
  */
#include "DeviceConfig.h"
#include "DeviceFiber.h"
#include "Timer.h"

#define INITIAL_STACK_DEPTH (fiber_initial_stack_base() - 0x04)

//Serial serial(USBTX, USBRX);

/*
 * Statically allocated values used to create and destroy Fibers.
 * required to be defined here to allow persistence during context switches.
 */
namespace codal
{
Fiber *currentFiber = NULL;                        // The context in which the current fiber is executing.
static Fiber *forkedFiber = NULL;                  // The context in which a newly created child fiber is executing.
static Fiber *idleFiber = NULL;                    // the idle task - performs a power efficient sleep, and system maintenance tasks.

/*
 * Scheduler state.
 */
static Fiber *runQueue = NULL;                     // The list of runnable fibers.
static Fiber *sleepQueue = NULL;                   // The list of blocked fibers waiting on a fiber_sleep() operation.
static Fiber *waitQueue = NULL;                    // The list of blocked fibers waiting on an event.
static Fiber *fiberPool = NULL;                    // Pool of unused fibers, just waiting for a job to do.

/*
 * Scheduler wide flags
 */
static uint8_t fiber_flags = 0;

/*
 * Fibers may perform wait/notify semantics on events. If set, these operations will be permitted on this EventModel.
 */
static EventModel *messageBus = NULL;
}

using namespace codal;

extern CodalDevice& device;

/**
  * Utility function to add the currenty running fiber to the given queue.
  *
  * Perform a simple add at the head, to avoid complexity,
  *
  * Queues are normally very short, so maintaining a doubly linked, sorted list typically outweighs the cost of
  * brute force searching.
  *
  * @param f The fiber to add to the queue
  *
  * @param queue The run queue to add the fiber to.
  */
void codal::queue_fiber(Fiber *f, Fiber **queue)
{
    device.disableInterrupts();

    // Record which queue this fiber is on.
    f->queue = queue;

    // Add the fiber to the tail of the queue. Although this involves scanning the
    // list, it results in fairer scheduling.
    if (*queue == NULL)
    {
        f->next = NULL;
        f->prev = NULL;
        *queue = f;
    }
    else
    {
        // Scan to the end of the queue.
        // We don't maintain a tail pointer to save RAM (queues are nrmally very short).
        Fiber *last = *queue;

        while (last->next != NULL)
            last = last->next;

        last->next = f;
        f->prev = last;
        f->next = NULL;
    }

    device.enableInterrupts();
}

/**
  * Utility function to the given fiber from whichever queue it is currently stored on.
  *
  * @param f the fiber to remove.
  */
void codal::dequeue_fiber(Fiber *f)
{
    // If this fiber is already dequeued, nothing the there's nothing to do.
    if (f->queue == NULL)
        return;

    // Remove this fiber fromm whichever queue it is on.
    device.disableInterrupts();

    if (f->prev != NULL)
        f->prev->next = f->next;
    else
        *(f->queue) = f->next;

    if(f->next)
        f->next->prev = f->prev;

    f->next = NULL;
    f->prev = NULL;
    f->queue = NULL;

    device.enableInterrupts();
}

/**
  * Allocates a fiber from the fiber pool if availiable. Otherwise, allocates a new one from the heap.
  */
Fiber *getFiberContext()
{
    Fiber *f;

    device.disableInterrupts();

    if (fiberPool != NULL)
    {
        f = fiberPool;
        dequeue_fiber(f);
    }
    else
    {
        f = new Fiber();

        if (f == NULL)
            return NULL;

        f->stack_bottom = 0;
        f->stack_top = 0;
    }

    device.enableInterrupts();

    // Ensure this fiber is in suitable state for reuse.
    f->flags = 0;

    tcb_configure_stack_base(&f->tcb, fiber_initial_stack_base());

    return f;
}


/**
  * Initialises the Fiber scheduler.
  * Creates a Fiber context around the calling thread, and adds it to the run queue as the current thread.
  *
  * This function must be called once only from the main thread, and before any other Fiber operation.
  *
  * @param _messageBus An event model, used to direct the priorities of the scheduler.
  */
void codal::scheduler_init(EventModel &_messageBus)
{
    // If we're already initialised, then nothing to do.
    if (fiber_scheduler_running())
        return;

        // Store a reference to the messageBus provided.
    // This parameter will be NULL if we're being run without a message bus.
    messageBus = &_messageBus;

    // Create a new fiber context
    currentFiber = getFiberContext();

    // Add ourselves to the run queue.
    queue_fiber(currentFiber, &runQueue);

    // Create the IDLE fiber.
    // Configure the fiber to directly enter the idle task.
    idleFiber = getFiberContext();

    tcb_configure_sp(&idleFiber->tcb, INITIAL_STACK_DEPTH);
    tcb_configure_lr(&idleFiber->tcb, (PROCESSOR_WORD_TYPE)&idle_task);

    if (messageBus)
    {
        // Register to receive events in the NOTIFY channel - this is used to implement wait-notify semantics
        messageBus->listen(DEVICE_ID_NOTIFY, DEVICE_EVT_ANY, scheduler_event, MESSAGE_BUS_LISTENER_IMMEDIATE);
        messageBus->listen(DEVICE_ID_NOTIFY_ONE, DEVICE_EVT_ANY, scheduler_event, MESSAGE_BUS_LISTENER_IMMEDIATE);

        system_timer_event_every_us(SCHEDULER_TICK_PERIOD_US, DEVICE_ID_SCHEDULER, DEVICE_SCHEDULER_EVT_TICK);
        messageBus->listen(DEVICE_ID_SCHEDULER, DEVICE_SCHEDULER_EVT_TICK, scheduler_tick, MESSAGE_BUS_LISTENER_IMMEDIATE);
    }

    fiber_flags |= DEVICE_SCHEDULER_RUNNING;
}

/**
  * Determines if the fiber scheduler is operational.
  *
  * @return 1 if the fber scheduler is running, 0 otherwise.
  */
int codal::fiber_scheduler_running()
{
    if (fiber_flags & DEVICE_SCHEDULER_RUNNING)
        return 1;

    return 0;
}

/**
  * The timer callback, called from interrupt context once every SYSTEM_TICK_PERIOD_MS milliseconds.
  * This function checks to determine if any fibers blocked on the sleep queue need to be woken up
  * and made runnable.
  */
void codal::scheduler_tick(DeviceEvent evt)
{
    Fiber *f = sleepQueue;
    Fiber *t;

    evt.timestamp /= 1000;

    // Check the sleep queue, and wake up any fibers as necessary.
    while (f != NULL)
    {
        t = f->next;

        if (evt.timestamp >= f->context)
        {
            // Wakey wakey!
            dequeue_fiber(f);
            queue_fiber(f,&runQueue);
        }

        f = t;
    }
}

/**
  * Event callback. Called from an instance of DeviceMessageBus whenever an event is raised.
  *
  * This function checks to determine if any fibers blocked on the wait queue need to be woken up
  * and made runnable due to the event.
  *
  * @param evt the event that has just been raised on an instance of DeviceMessageBus.
  */
void codal::scheduler_event(DeviceEvent evt)
{
    Fiber *f = waitQueue;
    Fiber *t;
    int notifyOneComplete = 0;

    // This should never happen.
    // It is however, safe to simply ignore any events provided, as if no messageBus if recorded,
    // no fibers are permitted to block on events.
    if (messageBus == NULL)
        return;

    // Check the wait queue, and wake up any fibers as necessary.
    while (f != NULL)
    {
        t = f->next;

        // extract the event data this fiber is blocked on.
        uint16_t id = f->context & 0xFFFF;
        uint16_t value = (f->context & 0xFFFF0000) >> 16;

        // Special case for the NOTIFY_ONE channel...
        if ((evt.source == DEVICE_ID_NOTIFY_ONE && id == DEVICE_ID_NOTIFY) && (value == DEVICE_EVT_ANY || value == evt.value))
        {
            if (!notifyOneComplete)
            {
                // Wakey wakey!
                dequeue_fiber(f);
                queue_fiber(f,&runQueue);
                notifyOneComplete = 1;
            }
        }

        // Normal case.
        else if ((id == DEVICE_ID_ANY || id == evt.source) && (value == DEVICE_EVT_ANY || value == evt.value))
        {
            // Wakey wakey!
            dequeue_fiber(f);
            queue_fiber(f,&runQueue);
        }

        f = t;
    }

    // Unregister this event, as we've woken up all the fibers with this match.
    if (evt.source != DEVICE_ID_NOTIFY && evt.source != DEVICE_ID_NOTIFY_ONE)
        messageBus->ignore(evt.source, evt.value, scheduler_event);
}


/**
  * Blocks the calling thread for the given period of time.
  * The calling thread will be immediateley descheduled, and placed onto a
  * wait queue until the requested amount of time has elapsed.
  *
  * @param t The period of time to sleep, in milliseconds.
  *
  * @note the fiber will not be be made runnable until after the elapsed time, but there
  * are no guarantees precisely when the fiber will next be scheduled.
  */
void codal::fiber_sleep(unsigned long t)
{
    Fiber *f = currentFiber;

    // If the scheduler is not running, then simply perform a spin wait and exit.
    if (!fiber_scheduler_running())
    {
        #warning "wait_ms needs to be implemented, commented out here"
        //wait_ms(t);
        return;
    }

    // Sleep is a blocking call, so if we're in a fork on block context,
    // it's time to spawn a new fiber...
    if (currentFiber->flags & DEVICE_FIBER_FLAG_FOB)
    {
        // Allocate a new fiber. This will come from the fiber pool if availiable,
        // else a new one will be allocated on the heap.
        forkedFiber = getFiberContext();

        // If we're out of memory, there's nothing we can do.
        // keep running in the context of the current thread as a best effort.
        if (forkedFiber != NULL)
            f = forkedFiber;
    }

    // Calculate and store the time we want to wake up.
    f->context = system_timer_current_time() + t;

    // Remove fiber from the run queue
    dequeue_fiber(f);

    // Add fiber to the sleep queue. We maintain strict ordering here to reduce lookup times.
    queue_fiber(f, &sleepQueue);

    // Finally, enter the scheduler.
    schedule();
}

/**
  * Blocks the calling thread until the specified event is raised.
  * The calling thread will be immediateley descheduled, and placed onto a
  * wait queue until the requested event is received.
  *
  * @param id The ID field of the event to listen for (e.g. DEVICE_ID_BUTTON_A)
  *
  * @param value The value of the event to listen for (e.g. DEVICE_BUTTON_EVT_CLICK)
  *
  * @return DEVICE_OK, or DEVICE_NOT_SUPPORTED if the fiber scheduler is not running, or associated with an EventModel.
  *
  * @code
  * fiber_wait_for_event(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK);
  * @endcode
  *
  * @note the fiber will not be be made runnable until after the event is raised, but there
  * are no guarantees precisely when the fiber will next be scheduled.
  */
int codal::fiber_wait_for_event(uint16_t id, uint16_t value)
{
    int ret = fiber_wake_on_event(id, value);

    if(ret == DEVICE_OK)
        schedule();

    return ret;
}

/**
  * Configures the fiber context for the current fiber to block on an event ID
  * and value, but does not deschedule the fiber.
  *
  * @param id The ID field of the event to listen for (e.g. DEVICE_ID_BUTTON_A)
  *
  * @param value The value of the event to listen for (e.g. DEVICE_BUTTON_EVT_CLICK)
  *
  * @return DEVICE_OK, or DEVICE_NOT_SUPPORTED if the fiber scheduler is not running, or associated with an EventModel.
  *
  * @code
  * fiber_wake_on_event(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK);
  *
  * //perform some time critical operation.
  *
  * //deschedule the current fiber manually, waiting for the previously configured event.
  * schedule();
  * @endcode
  */
int codal::fiber_wake_on_event(uint16_t id, uint16_t value)
{
    Fiber *f = currentFiber;

    if (messageBus == NULL || !fiber_scheduler_running())
        return DEVICE_NOT_SUPPORTED;

    // Sleep is a blocking call, so if we're in a fork on block context,
    // it's time to spawn a new fiber...
    if (currentFiber->flags & DEVICE_FIBER_FLAG_FOB)
    {
        // Allocate a TCB from the new fiber. This will come from the tread pool if availiable,
        // else a new one will be allocated on the heap.
        forkedFiber = getFiberContext();

        // If we're out of memory, there's nothing we can do.
        // keep running in the context of the current thread as a best effort.
        if (forkedFiber != NULL)
            f = forkedFiber;
    }

    // Encode the event data in the context field. It's handy having a 32 bit core. :-)
    f->context = (uint32_t)value << 16 | id;

    // Remove ourselves from the run queue
    dequeue_fiber(f);

    // Add ourselves to the sleep queue. We maintain strict ordering here to reduce lookup times.
    queue_fiber(f, &waitQueue);

    // Register to receive this event, so we can wake up the fiber when it happens.
    // Special case for the notify channel, as we always stay registered for that.
    if (id != DEVICE_ID_NOTIFY && id != DEVICE_ID_NOTIFY_ONE)
        messageBus->listen(id, value, scheduler_event, MESSAGE_BUS_LISTENER_IMMEDIATE);

    return DEVICE_OK;
}

/**
  * Executes the given function asynchronously if necessary.
  *
  * Fibers are often used to run event handlers, however many of these event handlers are very simple functions
  * that complete very quickly, bringing unecessary RAM overhead.
  *
  * This function takes a snapshot of the current processor context, then attempts to optimistically call the given function directly.
  * We only create an additional fiber if that function performs a block operation.
  *
  * @param entry_fn The function to execute.
  *
  * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
  */
int codal::invoke(void (*entry_fn)(void))
{
    // Validate our parameters.
    if (entry_fn == NULL)
        return DEVICE_INVALID_PARAMETER;

    if (!fiber_scheduler_running())
        return DEVICE_NOT_SUPPORTED;

    if (currentFiber->flags & DEVICE_FIBER_FLAG_FOB)
    {
        // If we attempt a fork on block whilst already in  fork n block context,
        // simply launch a fiber to deal with the request and we're done.
        create_fiber(entry_fn);
        return DEVICE_OK;
    }

    // Snapshot current context, but also update the Link Register to
    // refer to our calling function.
    save_register_context(&currentFiber->tcb);

    // If we're here, there are two possibilities:
    // 1) We're about to attempt to execute the user code
    // 2) We've already tried to execute the code, it blocked, and we've backtracked.

    // If we're returning from the user function and we forked another fiber then cleanup and exit.
    if (currentFiber->flags & DEVICE_FIBER_FLAG_PARENT)
    {
        currentFiber->flags &= ~DEVICE_FIBER_FLAG_FOB;
        currentFiber->flags &= ~DEVICE_FIBER_FLAG_PARENT;
        return DEVICE_OK;
    }

    // Otherwise, we're here for the first time. Enter FORK ON BLOCK mode, and
    // execute the function directly. If the code tries to block, we detect this and
    // spawn a thread to deal with it.
    currentFiber->flags |= DEVICE_FIBER_FLAG_FOB;
    entry_fn();
    currentFiber->flags &= ~DEVICE_FIBER_FLAG_FOB;

    // If this is is an exiting fiber that for spawned to handle a blocking call, recycle it.
    // The fiber will then re-enter the scheduler, so no need for further cleanup.
    if (currentFiber->flags & DEVICE_FIBER_FLAG_CHILD)
        release_fiber();

     return DEVICE_OK;
}

/**
  * Executes the given function asynchronously if necessary, and offers the ability to provide a parameter.
  *
  * Fibers are often used to run event handlers, however many of these event handlers are very simple functions
  * that complete very quickly, bringing unecessary RAM. overhead
  *
  * This function takes a snapshot of the current fiber context, then attempt to optimistically call the given function directly.
  * We only create an additional fiber if that function performs a block operation.
  *
  * @param entry_fn The function to execute.
  *
  * @param param an untyped parameter passed into the entry_fn and completion_fn.
  *
  * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
  */
int codal::invoke(void (*entry_fn)(void *), void *param)
{
    // Validate our parameters.
    if (entry_fn == NULL)
        return DEVICE_INVALID_PARAMETER;

    if (!fiber_scheduler_running())
        return DEVICE_NOT_SUPPORTED;

    if (currentFiber->flags & (DEVICE_FIBER_FLAG_FOB | DEVICE_FIBER_FLAG_PARENT | DEVICE_FIBER_FLAG_CHILD))
    {
        // If we attempt a fork on block whilst already in a fork on block context,
        // simply launch a fiber to deal with the request and we're done.
        create_fiber(entry_fn, param);
        return DEVICE_OK;
    }

    //Serial.println("BEFORE SAVE");
    //while (!(UCSR0A & _BV(TXC0)));

    // Snapshot current context, but also update the Link Register to
    // refer to our calling function.
    save_register_context(&currentFiber->tcb);

    //Serial.println("AFTER SAVE");
    //while (!(UCSR0A & _BV(TXC0)));

    // If we're here, there are two possibilities:
    // 1) We're about to attempt to execute the user code
    // 2) We've already tried to execute the code, it blocked, and we've backtracked.

    // If we're returning from the user function and we forked another fiber then cleanup and exit.
    if (currentFiber->flags & DEVICE_FIBER_FLAG_PARENT)
    {
        currentFiber->flags &= ~DEVICE_FIBER_FLAG_FOB;
        currentFiber->flags &= ~DEVICE_FIBER_FLAG_PARENT;
        return DEVICE_OK;
    }

    // Otherwise, we're here for the first time. Enter FORK ON BLOCK mode, and
    // execute the function directly. If the code tries to block, we detect this and
    // spawn a thread to deal with it.
    currentFiber->flags |= DEVICE_FIBER_FLAG_FOB;
    entry_fn(param);
    currentFiber->flags &= ~DEVICE_FIBER_FLAG_FOB;

    // If this is is an exiting fiber that for spawned to handle a blocking call, recycle it.
    // The fiber will then re-enter the scheduler, so no need for further cleanup.
    if (currentFiber->flags & DEVICE_FIBER_FLAG_CHILD)
        release_fiber(param);

    return DEVICE_OK;
}

/**
 * Launches a fiber.
 *
 * @param ep the entry point for the fiber.
 *
 * @param cp the completion routine after ep has finished execution
 */
void codal::launch_new_fiber(void (*ep)(void), void (*cp)(void))
{
    // Execute the thread's entrypoint
    ep();

    // Execute the thread's completion routine;
    cp();

    // If we get here, then the completion routine didn't recycle the fiber... so do it anyway. :-)
    release_fiber();
}

/**
 * Launches a fiber with a parameter
 *
 * @param ep the entry point for the fiber.
 *
 * @param cp the completion routine after ep has finished execution
 *
 * @param pm the parameter to provide to ep and cp.
 */
void codal::launch_new_fiber_param(void (*ep)(void *), void (*cp)(void *), void *pm)
{
    // Execute the thread's entrypoint.
    ep(pm);

    // Execute the thread's completion routine.
    cp(pm);

    // If we get here, then the completion routine didn't recycle the fiber... so do it anyway. :-)
    release_fiber(pm);
}


Fiber *__create_fiber(uint32_t ep, uint32_t cp, uint32_t pm, int parameterised)
{
    // Validate our parameters.
    if (ep == 0 || cp == 0)
        return NULL;

    // Allocate a TCB from the new fiber. This will come from the fiber pool if availiable,
    // else a new one will be allocated on the heap.
    Fiber *newFiber = getFiberContext();

    // If we're out of memory, there's nothing we can do.
    if (newFiber == NULL)
        return NULL;

    tcb_configure_args(&newFiber->tcb, ep, cp, pm);
    tcb_configure_sp(&newFiber->tcb, INITIAL_STACK_DEPTH);
    tcb_configure_lr(&newFiber->tcb, parameterised ? (PROCESSOR_WORD_TYPE) &launch_new_fiber_param : (PROCESSOR_WORD_TYPE) &launch_new_fiber);

    // Add new fiber to the run queue.
    queue_fiber(newFiber, &runQueue);

    return newFiber;
}

/**
  * Creates a new Fiber, and launches it.
  *
  * @param entry_fn The function the new Fiber will begin execution in.
  *
  * @param completion_fn The function called when the thread completes execution of entry_fn.
  *                      Defaults to release_fiber.
  *
  * @return The new Fiber, or NULL if the operation could not be completed.
  */
Fiber *codal::create_fiber(void (*entry_fn)(void), void (*completion_fn)(void))
{
    if (!fiber_scheduler_running())
        return NULL;

    return __create_fiber((uint32_t) entry_fn, (uint32_t)completion_fn, 0, 0);
}


/**
  * Creates a new parameterised Fiber, and launches it.
  *
  * @param entry_fn The function the new Fiber will begin execution in.
  *
  * @param param an untyped parameter passed into the entry_fn and completion_fn.
  *
  * @param completion_fn The function called when the thread completes execution of entry_fn.
  *                      Defaults to release_fiber.
  *
  * @return The new Fiber, or NULL if the operation could not be completed.
  */
Fiber *codal::create_fiber(void (*entry_fn)(void *), void *param, void (*completion_fn)(void *))
{
    if (!fiber_scheduler_running())
        return NULL;

    return __create_fiber((uint32_t) entry_fn, (uint32_t)completion_fn, (uint32_t) param, 1);
}

/**
  * Exit point for all fibers.
  *
  * Any fiber reaching the end of its entry function will return here  for recycling.
  */
void codal::release_fiber(void *)
{
    if (!fiber_scheduler_running())
        return;

    release_fiber();
}

/**
  * Exit point for all fibers.
  *
  * Any fiber reaching the end of its entry function will return here  for recycling.
  */
void codal::release_fiber(void)
{
    if (!fiber_scheduler_running())
        return;

    // Remove ourselves form the runqueue.
    dequeue_fiber(currentFiber);

    // Add ourselves to the list of free fibers
    queue_fiber(currentFiber, &fiberPool);

    // Find something else to do!
    schedule();
}

/**
  * Resizes the stack allocation of the current fiber if necessary to hold the system stack.
  *
  * If the stack allocation is large enough to hold the current system stack, then this function does nothing.
  * Otherwise, the the current allocation of the fiber is freed, and a larger block is allocated.
  *
  * @param f The fiber context to verify.
  *
  * @return The stack depth of the given fiber.
  */
void codal::verify_stack_size(Fiber *f)
{
    // Ensure the stack buffer is large enough to hold the stack Reallocate if necessary.
    PROCESSOR_WORD_TYPE stackDepth;
    PROCESSOR_WORD_TYPE bufferSize;

    // Calculate the stack depth.
    stackDepth = tcb_get_stack_base(&f->tcb) - (PROCESSOR_WORD_TYPE)get_current_sp();

    // Calculate the size of our allocated stack buffer
    bufferSize = f->stack_top - f->stack_bottom;

    // If we're too small, increase our buffer size.
    if (bufferSize < stackDepth)
    {
        // To ease heap churn, we choose the next largest multple of 32 bytes.
        bufferSize = (stackDepth + 32) & 0xffffffe0;

        // Release the old memory
        if (f->stack_bottom != 0)
            free((void *)f->stack_bottom);

        // Allocate a new one of the appropriate size.
        f->stack_bottom = (PROCESSOR_WORD_TYPE)malloc(bufferSize);

        // Recalculate where the top of the stack is and we're done.
        f->stack_top = f->stack_bottom + bufferSize;
    }
}

/**
  * Determines if any fibers are waiting to be scheduled.
  *
  * @return The number of fibers currently on the run queue
  */
int codal::scheduler_runqueue_empty()
{
    return (runQueue == NULL);
}

/**
  * Calls the Fiber scheduler.
  * The calling Fiber will likely be blocked, and control given to another waiting fiber.
  * Call this function to yield control of the processor when you have nothing more to do.
  */
void codal::schedule()
{
    if (!fiber_scheduler_running())
        return;

    // First, take a reference to the currently running fiber;
    Fiber *oldFiber = currentFiber;

    // First, see if we're in Fork on Block context. If so, we simply want to store the full context
    // of the currently running thread in a newly created fiber, and restore the context of the
    // currently running fiber, back to the point where it entered FOB.

    if (currentFiber->flags & DEVICE_FIBER_FLAG_FOB)
    {
        // Record that the fibers have a parent/child relationship
        currentFiber->flags |= DEVICE_FIBER_FLAG_PARENT;
        forkedFiber->flags |= DEVICE_FIBER_FLAG_CHILD;

        // Define the stack base of the forked fiber to be align with the entry point of the parent fiber
        tcb_configure_stack_base(&forkedFiber->tcb, tcb_get_sp(&currentFiber->tcb));

        // Ensure the stack allocation of the new fiber is large enough
        verify_stack_size(forkedFiber);

        // Store the full context of this fiber.
        save_context(&forkedFiber->tcb, forkedFiber->stack_top);

        // We may now be either the newly created thread, or the one that created it.
        // if the DEVICE_FIBER_FLAG_PARENT flag is still set, we're the old thread, so
        // restore the current fiber to its stored context and we're done.
        if (currentFiber->flags & DEVICE_FIBER_FLAG_PARENT)
            restore_register_context(&currentFiber->tcb);

        // If we're the new thread, we must have been unblocked by the scheduler, so simply return
        // and continue processing.
        return;
    }

    // We're in a normal scheduling context, so perform a round robin algorithm across runnable fibers.
    // OK - if we've nothing to do, then run the IDLE task (power saving sleep)
    if (runQueue == NULL)
        currentFiber = idleFiber;

    else if (currentFiber->queue == &runQueue)
        // If the current fiber is on the run queue, round robin.
        currentFiber = currentFiber->next == NULL ? runQueue : currentFiber->next;

    else
        // Otherwise, just pick the head of the run queue.
        currentFiber = runQueue;

    if (currentFiber == idleFiber && oldFiber->flags & DEVICE_FIBER_FLAG_DO_NOT_PAGE)
    {
        // Run the idle task right here using the old fiber's stack.
        // Keep idling while the runqueue is empty, or there is data to process.

        // Run in the context of the original fiber, to preserve state of flags...
        // as we are running on top of this fiber's stack.
        currentFiber = oldFiber;

        do
        {
            idle();
        }
        while (runQueue == NULL);

        // Switch to a non-idle fiber.
        // If this fiber is the same as the old one then there'll be no switching at all.
        currentFiber = runQueue;
    }

    // Swap to the context of the chosen fiber, and we're done.
    // Don't bother with the overhead of switching if there's only one fiber on the runqueue!
    if (currentFiber != oldFiber)
    {

        // Special case for the idle task, as we don't maintain a stack context (just to save memory).
        if (currentFiber == idleFiber)
        {
            tcb_configure_sp(&idleFiber->tcb, INITIAL_STACK_DEPTH);
            tcb_configure_lr(&idleFiber->tcb, (PROCESSOR_WORD_TYPE)&idle_task);
        }

        if (oldFiber == idleFiber)
        {
            // Just swap in the new fiber, and discard changes to stack and register context.
            swap_context(NULL, &currentFiber->tcb, 0, currentFiber->stack_top);
        }
        else
        {
            // Ensure the stack allocation of the fiber being scheduled out is large enough
            verify_stack_size(oldFiber);

            // Schedule in the new fiber.
            swap_context(&oldFiber->tcb, &currentFiber->tcb, oldFiber->stack_top, currentFiber->stack_top);
        }
    }
}

/**
  * Set of tasks to perform when idle.
  * Service any background tasks that are required, and attempt a power efficient sleep.
  */
void codal::idle()
{
    // Prevent an idle loop of death:
    // We will return to idle after processing any idle events that add anything
    // to our run queue, we use the DEVICE_SCHEDULER_IDLE flag to determine this
    // scenario.
    if(!(fiber_flags & DEVICE_SCHEDULER_IDLE))
    {
        fiber_flags |= DEVICE_SCHEDULER_IDLE;
        DeviceEvent(DEVICE_ID_SCHEDULER, DEVICE_SCHEDULER_EVT_IDLE);
    }

    // If the above did create any useful work, enter power efficient sleep.
    if(scheduler_runqueue_empty())
    {
        // unset our DEVICE_SCHEDULER_IDLE flag, we have processed all of the events
        // because we enforce MESSAGE_BUS_LISTENER_IMMEDIATE for listeners placed
        // on the scheduler.
        fiber_flags &= ~DEVICE_SCHEDULER_IDLE;
        device.waitForEvent();
    }
}

/**
  * The idle task, which is called when the runtime has no fibers that require execution.
  *
  * This function typically calls idle().
  */
void codal::idle_task()
{
    while(1)
    {
        idle();
        schedule();
    }
}
