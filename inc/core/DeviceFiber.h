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
  *
  * TODO: Consider a split mode scheduler, that monitors used stack size, and maintains a dedicated, persistent
  * stack for any long lived fibers with large stack
  */
#ifndef DEVICE_FIBER_H
#define DEVICE_FIBER_H

#include "DeviceConfig.h"
#include "DeviceEvent.h"
#include "EventModel.h"
#include "device_fiber.h"

// Fiber Scheduler Flags
#define DEVICE_SCHEDULER_RUNNING            0x01
#define DEVICE_SCHEDULER_IDLE               0x02

// Fiber Flags
#define DEVICE_FIBER_FLAG_FOB               0x01
#define DEVICE_FIBER_FLAG_PARENT            0x02
#define DEVICE_FIBER_FLAG_CHILD             0x04
#define DEVICE_FIBER_FLAG_DO_NOT_PAGE       0x08

#define DEVICE_SCHEDULER_EVT_TICK           1
#define DEVICE_SCHEDULER_EVT_IDLE           2

namespace codal
{
    /**
      * Representation of a single Fiber
      */
    struct Fiber
    {
        PROCESSOR_TCB tcb;                  // Thread context when last scheduled out.
        PROCESSOR_WORD_TYPE stack_bottom;              // The start address of this Fiber's stack. The stack is heap allocated, and full descending.
        PROCESSOR_WORD_TYPE stack_top;                 // The end address of this Fiber's stack.
        uint32_t context;                   // Context specific information.
        uint32_t flags;                     // Information about this fiber.
        Fiber **queue;                      // The queue this fiber is stored on.
        Fiber *next, *prev;                 // Position of this Fiber on the run queue.
    };

    extern Fiber *currentFiber;

    /**
      * Initialises the Fiber scheduler.
      * Creates a Fiber context around the calling thread, and adds it to the run queue as the current thread.
      *
      * This function must be called once only from the main thread, and before any other Fiber operation.
      *
      * @param _messageBus An event model, used to direct the priorities of the scheduler.
      */
    void scheduler_init(EventModel &_messageBus);

    /**
      * Determines if the fiber scheduler is operational.
      *
      * @return 1 if the fber scheduler is running, 0 otherwise.
      */
    int fiber_scheduler_running();

    /**
      * Exit point for all fibers.
      *
      * Any fiber reaching the end of its entry function will return here  for recycling.
      */
    void release_fiber(void);
    void release_fiber(void *param);

    /**
     * Launches a fiber.
     *
     * @param ep the entry point for the fiber.
     *
     * @param cp the completion routine after ep has finished execution
     */
    void launch_new_fiber(void (*ep)(void), void (*cp)(void))
    #ifdef __GCC__
        __attribute__((naked))
    #endif
    ;

    /**
     * Launches a fiber with a parameter
     *
     * @param ep the entry point for the fiber.
     *
     * @param cp the completion routine after ep has finished execution
     *
     * @param pm the parameter to provide to ep and cp.
     */
    void launch_new_fiber_param(void (*ep)(void *), void (*cp)(void *), void *pm)
    #ifdef __GCC__
        __attribute__((naked))
    #endif
    ;

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
    Fiber *create_fiber(void (*entry_fn)(void), void (*completion_fn)(void) = release_fiber);


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
    Fiber *create_fiber(void (*entry_fn)(void *), void *param, void (*completion_fn)(void *) = release_fiber);


    /**
      * Calls the Fiber scheduler.
      * The calling Fiber will likely be blocked, and control given to another waiting fiber.
      * Call this function to yield control of the processor when you have nothing more to do.
      */
    void schedule()
    #ifdef __GCC__
        __attribute__((naked))
    #endif
    ;

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
    void fiber_sleep(unsigned long t);

    /**
      * The timer callback, called from interrupt context once every SYSTEM_TICK_PERIOD_MS milliseconds.
      * This function checks to determine if any fibers blocked on the sleep queue need to be woken up
      * and made runnable.
      */
    void scheduler_tick(DeviceEvent);

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
    int fiber_wait_for_event(uint16_t id, uint16_t value);

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
    int fiber_wake_on_event(uint16_t id, uint16_t value);

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
    int invoke(void (*entry_fn)(void));

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
    int invoke(void (*entry_fn)(void *), void *param);

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
    inline void verify_stack_size(Fiber *f);

    /**
      * Event callback. Called from an instance of DeviceMessageBus whenever an event is raised.
      *
      * This function checks to determine if any fibers blocked on the wait queue need to be woken up
      * and made runnable due to the event.
      *
      * @param evt the event that has just been raised on an instance of DeviceMessageBus.
      */
    void scheduler_event(DeviceEvent evt);

    /**
      * Determines if any fibers are waiting to be scheduled.
      *
      * @return The number of fibers currently on the run queue
      */
    int scheduler_runqueue_empty();

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
    void queue_fiber(Fiber *f, Fiber **queue);

    /**
      * Utility function to the given fiber from whichever queue it is currently stored on.
      *
      * @param f the fiber to remove.
      */
    void dequeue_fiber(Fiber *f);

    /**
      * Set of tasks to perform when idle.
      * Service any background tasks that are required, and attempt a power efficient sleep.
      */
    void idle();

    /**
      * The idle task, which is called when the runtime has no fibers that require execution.
      *
      * This function typically calls idle().
      */
    void idle_task();
}


/**
  * Assembler Context switch routing.
  * Defined in CortexContextSwitch.s.
  */
extern "C" void swap_context(PROCESSOR_TCB *from, PROCESSOR_TCB *to, PROCESSOR_WORD_TYPE from_stack, PROCESSOR_WORD_TYPE to_stack);
extern "C" void save_context(PROCESSOR_TCB *tcb, PROCESSOR_WORD_TYPE stack);
extern "C" void save_register_context(PROCESSOR_TCB *tcb);
extern "C" void restore_register_context(PROCESSOR_TCB *tcb);

#endif
