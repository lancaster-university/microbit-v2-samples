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
  * A simple 32 bit block based memory allocator. This allows one or more memory segments to
  * be designated as heap storage, and is designed to run in a static memory area or inside the standard C
  * heap for use by the codal device runtime. This is required for several reasons:
  *
  * 1) It reduces memory fragmentation due to the high churn sometime placed on the heap
  * by ManagedTypes, fibers and user code. Underlying heap implentations are often have very simplistic
  * allocation pilicies and suffer from fragmentation in prolonged use - which can cause programs to
  * stop working after a period of time. The algorithm implemented here is simple, but highly tolerant to
  * large amounts of churn.
  *
  * 2) It allows us to reuse the 8K of SRAM set aside for SoftDevice as additional heap storage
  * when BLE is not in use.
  *
  * 3) It gives a simple example of how memory allocation works! :-)
  *
  * P.S. This is a very simple allocator, therefore not without its weaknesses. Why don't you consider
  * what these are, and consider the tradeoffs against simplicity...
  *
  * @note The need for this should be reviewed in the future, if a different memory allocator is
  * made availiable in the mbed platform.
  *
  * TODO: Consider caching recently freed blocks to improve allocation time.
  */

#include "DeviceConfig.h"
#include "DeviceHeapAllocator.h"
#include "CodalDevice.h"
#include "CodalCompat.h"
#include "ErrorNo.h"
#include <malloc.h>

#if CONFIG_ENABLED(DEVICE_HEAP_ALLOCATOR)

// A list of all active heap regions, and their dimensions in memory.
struct HeapDefinition
{
    uint32_t *heap_start;		// Physical address of the start of this heap.
    uint32_t *heap_end;		    // Physical address of the end of this heap.
};

HeapDefinition heap[DEVICE_MAXIMUM_HEAPS] = { };
uint8_t heap_count = 0;


#if CONFIG_ENABLED(DEVICE_DBG) && CONFIG_ENABLED(DEVICE_HEAP_DBG)
// Diplays a usage summary about a given heap...
void device_heap_print(HeapDefinition &heap)
{
    uint32_t	blockSize;
    uint32_t	*block;
    int         totalFreeBlock = 0;
    int         totalUsedBlock = 0;
    int         cols = 0;

    if (heap.heap_start == NULL)
    {
        if(SERIAL_DEBUG) SERIAL_DEBUG->printf("--- HEAP NOT INITIALISED ---\n");
        return;
    }

    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("heap_start : %p\n", heap.heap_start);
    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("heap_end   : %p\n", heap.heap_end);
    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("heap_size  : %d\n", (int)heap.heap_end - (int)heap.heap_start);

    // Disable IRQ temporarily to ensure no race conditions!
    device.disableInterrupts();

    block = heap.heap_start;
    while (block < heap.heap_end)
    {
    blockSize = *block & ~DEVICE_HEAP_BLOCK_FREE;
        if(SERIAL_DEBUG) SERIAL_DEBUG->printf("[%c:%d] ", *block & DEVICE_HEAP_BLOCK_FREE ? 'F' : 'U', blockSize*4);
        if (cols++ == 20)
        {
            if(SERIAL_DEBUG) SERIAL_DEBUG->printf("\n");
            cols = 0;
        }

        if (*block & DEVICE_HEAP_BLOCK_FREE)
            totalFreeBlock += blockSize;
        else
            totalUsedBlock += blockSize;

    block += blockSize;
    }

    // Enable Interrupts
    device.enableInterrupts();

    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("\n");

    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("mb_total_free : %d\n", totalFreeBlock*4);
    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("mb_total_used : %d\n", totalUsedBlock*4);
}


// Diagnostics function. Displays a usage summary about all initialised heaps.
void device_heap_print()
{
    for (int i=0; i < heap_count; i++)
    {
        if(SERIAL_DEBUG) SERIAL_DEBUG->printf("\nHEAP %d: \n", i);
        device_heap_print(heap[i]);
    }
}
#endif

void device_initialise_heap(HeapDefinition &heap)
{
    // Simply mark the entire heap as free.
    *heap.heap_start = ((uint32_t) heap.heap_end - (uint32_t) heap.heap_start) / DEVICE_HEAP_BLOCK_SIZE;
    *heap.heap_start |= DEVICE_HEAP_BLOCK_FREE;
}

/**
  * Create and initialise a given memory region as for heap storage.
  * After this is called, any future calls to malloc, new, free or delete may use the new heap.
  * The heap allocator will attempt to allocate memory from heaps in the order that they are created.
  * i.e. memory will be allocated from first heap created until it is full, then the second heap, and so on.
  *
  * @param start The start address of memory to use as a heap region.
  *
  * @param end The end address of memory to use as a heap region.
  *
  * @return DEVICE_OK on success, or DEVICE_NO_RESOURCES if the heap could not be allocated.
  *
  * @note Only code that #includes DeviceHeapAllocator.h will use this heap. This includes all codal device runtime
  * code, and user code targetting the runtime. External code can choose to include this file, or
  * simply use the standard heap.
  */
int device_create_heap(uint32_t start, uint32_t end)
{
    // Ensure we don't exceed the maximum number of heap segments.
    if (heap_count == DEVICE_MAXIMUM_HEAPS)
        return DEVICE_NO_RESOURCES;

    // Sanity check. Ensure range is valid, large enough and word aligned.
    if (end <= start || end - start < DEVICE_HEAP_BLOCK_SIZE*2 || end % 4 != 0 || start % 4 != 0)
        return DEVICE_INVALID_PARAMETER;

    // Disable IRQ temporarily to ensure no race conditions!
    device.disableInterrupts();

    // Record the dimensions of this new heap
    heap[heap_count].heap_start = (uint32_t *)start;
    heap[heap_count].heap_end = (uint32_t *)end;

    // Initialise the heap as being completely empty and available for use.
    device_initialise_heap(heap[heap_count]);
    heap_count++;

    // Enable Interrupts
    device.enableInterrupts();

#if CONFIG_ENABLED(DEVICE_DBG) && CONFIG_ENABLED(DEVICE_HEAP_DBG)
    device_heap_print();
#endif

    return DEVICE_OK;
}

/**
  * Attempt to allocate a given amount of memory from a given heap area.
  *
  * @param size The amount of memory, in bytes, to allocate.
  * @param heap The heap to allocate memory from.
  *
  * @return A pointer to the allocated memory, or NULL if insufficient memory is available.
  */
void *device_malloc(size_t size, HeapDefinition &heap)
{
    uint32_t	blockSize = 0;
    uint32_t	blocksNeeded = size % DEVICE_HEAP_BLOCK_SIZE == 0 ? size / DEVICE_HEAP_BLOCK_SIZE : size / DEVICE_HEAP_BLOCK_SIZE + 1;
    uint32_t	*block;
    uint32_t	*next;

    if (size <= 0)
    return NULL;

    // Account for the index block;
    blocksNeeded++;

    // Disable IRQ temporarily to ensure no race conditions!
    device.disableInterrupts();

    // We implement a first fit algorithm with cache to handle rapid churn...
    // We also defragment free blocks as we search, to optimise this and future searches.
    block = heap.heap_start;
    while (block < heap.heap_end)
    {
        // If the block is used, then keep looking.
        if(!(*block & DEVICE_HEAP_BLOCK_FREE))
        {
            block += *block;
            continue;
        }

        blockSize = *block & ~DEVICE_HEAP_BLOCK_FREE;

        // We have a free block. Let's see if the subsequent ones are too. If so, we can merge...
        next = block + blockSize;

        while (*next & DEVICE_HEAP_BLOCK_FREE)
        {
            if (next >= heap.heap_end)
                break;

            // We can merge!
            blockSize += (*next & ~DEVICE_HEAP_BLOCK_FREE);
            *block = blockSize | DEVICE_HEAP_BLOCK_FREE;

            next = block + blockSize;
        }

        // We have a free block. Let's see if it's big enough.
        // If so, we have a winner.
        if (blockSize >= blocksNeeded)
            break;

        // Otherwise, keep looking...
        block += blockSize;
    }

    // We're full!
    if (block >= heap.heap_end)
    {
        device.enableInterrupts();
        return NULL;
    }

    // If we're at the end of memory or have very near match then mark the whole segment as in use.
    if (blockSize <= blocksNeeded+1 || block+blocksNeeded+1 >= heap.heap_end)
    {
        // Just mark the whole block as used.
        *block &= ~DEVICE_HEAP_BLOCK_FREE;
    }
    else
    {
    // We need to split the block.
    uint32_t *splitBlock = block + blocksNeeded;
    *splitBlock = blockSize - blocksNeeded;
    *splitBlock |= DEVICE_HEAP_BLOCK_FREE;

    *block = blocksNeeded;
    }

    // Enable Interrupts
    device.enableInterrupts();

    return block+1;
}

/**
  * Attempt to allocate a given amount of memory from any of our configured heap areas.
  *
  * @param size The amount of memory, in bytes, to allocate.
  *
  * @return A pointer to the allocated memory, or NULL if insufficient memory is available.
  */
void* malloc (size_t size)
{
    static uint8_t initialised = 0;
    void *p;

    if (!initialised)
    {
        device_create_heap(CODAL_HEAP_START, CODAL_HEAP_END);
        initialised = 1;
    }

    // Assign the memory from the first heap created that has space.
    for (int i=0; i < heap_count; i++)
    {
        p = device_malloc(size, heap[i]);
        if (p != NULL)
        {
#if CONFIG_ENABLED(DEVICE_DBG) && CONFIG_ENABLED(DEVICE_HEAP_DBG)
            if(SERIAL_DEBUG) SERIAL_DEBUG->printf("device_malloc: ALLOCATED: %d [%p]\n", size, p);
#endif
            return p;
        }
    }

    // We're totally out of options (and memory!).
#if CONFIG_ENABLED(DEVICE_DBG) && CONFIG_ENABLED(DEVICE_HEAP_DBG)
    // Keep everything transparent if we've not been initialised yet
    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("device_malloc: OUT OF MEMORY [%d]\n", size);
#endif

#if CONFIG_ENABLED(DEVICE_PANIC_HEAP_FULL)
    device.panic(DEVICE_OOM);
#endif

    return NULL;
}

/**
  * Release a given area of memory from the heap.
  *
  * @param mem The memory area to release.
  */
void free (void *mem)
{
    uint32_t	*memory = (uint32_t *)mem;
    uint32_t	*cb = memory-1;

#if CONFIG_ENABLED(DEVICE_DBG) && CONFIG_ENABLED(DEVICE_HEAP_DBG)
    if (heap_count > 0)
        if(SERIAL_DEBUG) SERIAL_DEBUG->printf("device_free:   %p\n", mem);
#endif
    // Sanity check.
    if (memory == NULL)
       return;

    // If this memory was created from a heap registered with us, free it.
    for (int i=0; i < heap_count; i++)
    {
        if(memory > heap[i].heap_start && memory < heap[i].heap_end)
        {
            // The memory block given is part of this heap, so we can simply
            // flag that this memory area is now free, and we're done.
            if (*cb == 0 || *cb & DEVICE_HEAP_BLOCK_FREE)
                device.panic(DEVICE_HEAP_ERROR);
            *cb |= DEVICE_HEAP_BLOCK_FREE;
            return;
        }
    }

    // If we reach here, then the memory is not part of any registered heap.
    device.panic(DEVICE_HEAP_ERROR);
}

void* calloc (size_t num, size_t size)
{
    void *mem = malloc(num*size);

    if (mem)
        memclr(mem, num*size);

    return mem;
}

void* realloc (void* ptr, size_t size)
{
    void *mem = malloc(size);

    // handle the simplest case - no previous memory allocted.
    if (ptr != NULL && mem != NULL)
    {

        // Otherwise we need to copy and free up the old data.
        uint32_t *cb = ((uint32_t *)ptr) - 1;
        uint32_t blockSize = *cb & ~DEVICE_HEAP_BLOCK_FREE;

        memcpy(mem, ptr, min(blockSize, size));
        free(ptr);
    }

    return mem;
}

// make sure the libc allocator is not pulled in

void *_malloc_r(struct _reent *, size_t len)
{
    return malloc(len);
}

void _free_r(struct _reent *, void *addr)
{
    free(addr);
}

#endif
