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
  * Compile time configuration options for the codal device runtime.
  */

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include "yotta_cfg_mappings.h"
#include "common_includes.h"

//
// Memory configuration
//
// The start address of usable RAM memory.
#ifndef DEVICE_SRAM_BASE
#define DEVICE_SRAM_BASE                      0x20000000
#endif

// Physical address of the top of SRAM.
#ifndef DEVICE_SRAM_END
#define DEVICE_SRAM_END                       0x20008000
#endif

// The end address of memory normally reserved for Soft Device.
#ifndef DEVICE_SD_LIMIT
#define DEVICE_SD_LIMIT                       0x20002000
#endif

// The physical address in memory of the Soft Device GATT table.
#ifndef DEVICE_SD_GATT_TABLE_START
#define DEVICE_SD_GATT_TABLE_START            0x20001900
#endif

// Physical address of the top of the system stack (on mbed-classic this is the top of SRAM)
#ifndef CORTEX_M0_STACK_BASE
#define CORTEX_M0_STACK_BASE                    DEVICE_SRAM_END
#endif

// Amount of memory reserved for the stack at the end of memory (bytes).
#ifndef DEVICE_STACK_SIZE
#define DEVICE_STACK_SIZE                     2048
#endif

// Physical address of the end of heap space.
#ifndef CODAL_HEAP_START
                                              extern uint32_t __end__;
#define CODAL_HEAP_START                      (uint32_t)(&__end__)
#endif

// Physical address of the end of heap space.
#ifndef CODAL_HEAP_END
#define CODAL_HEAP_END                        (CORTEX_M0_STACK_BASE - DEVICE_STACK_SIZE)
#endif

// Enables or disables the DeviceHeapllocator. Note that if disabled, no reuse of the SRAM normally
// reserved for SoftDevice is possible, and out of memory condition will no longer be trapped...
// i.e. panic() will no longer be triggered on memory full conditions.
#ifndef DEVICE_HEAP_ALLOCATOR
#define DEVICE_HEAP_ALLOCATOR                 1
#endif

// Block size used by the allocator in bytes.
// n.b. Currently only 32 bits (4 bytes) is supported.
#ifndef DEVICE_HEAP_BLOCK_SIZE
#define DEVICE_HEAP_BLOCK_SIZE                4
#endif

// The proportion of SRAM available on the mbed heap to reserve for the codal device heap.
#ifndef DEVICE_NESTED_HEAP_SIZE
#define DEVICE_NESTED_HEAP_SIZE               0.75
#endif

// If defined, reuse any unused SRAM normally reserved for SoftDevice (Nordic's memory resident BLE stack) as heap memory.
// The amount of memory reused depends upon whether or not BLE is enabled using DEVICE_BLE_ENABLED.
// Set '1' to enable.
#ifndef DEVICE_HEAP_REUSE_SD
#define DEVICE_HEAP_REUSE_SD                  1
#endif

// The amount of memory allocated to Soft Device to hold its BLE GATT table.
// For standard S110 builds, this should be word aligned and in the range 0x300 - 0x700.
// Any unused memory will be automatically reclaimed as HEAP memory if both DEVICE_HEAP_REUSE_SD and DEVICE_HEAP_ALLOCATOR are enabled.
#ifndef DEVICE_SD_GATT_TABLE_SIZE
#define DEVICE_SD_GATT_TABLE_SIZE             0x300
#endif

//
// Fiber scheduler configuration
//

// Scheduling quantum (milliseconds)
// Also used to drive the codal device runtime system ticker.
#ifndef SCHEDULER_TICK_PERIOD_US
#define SCHEDULER_TICK_PERIOD_US                   6000
#endif

//
// Message Bus:
// Default behaviour for event handlers, if not specified in the listen() call
//
// Permissable values are:
//   MESSAGE_BUS_LISTENER_REENTRANT
//   MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY
//   MESSAGE_BUS_LISTENER_DROP_IF_BUSY
//   MESSAGE_BUS_LISTENER_IMMEDIATE

#ifndef EVENT_LISTENER_DEFAULT_FLAGS
#define EVENT_LISTENER_DEFAULT_FLAGS            MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY
#endif

//
// Maximum event queue depth. If a queue exceeds this depth, further events will be dropped.
// Used to prevent message queues growing uncontrollably due to badly behaved user code and causing panic conditions.
//
#ifndef MESSAGE_BUS_LISTENER_MAX_QUEUE_DEPTH
#define MESSAGE_BUS_LISTENER_MAX_QUEUE_DEPTH    10
#endif

//
// BLE options
//
// The BLE stack is very memory hungry. Each service can therefore be compiled in or out
// by enabling/disabling the options below.
//
// n.b. The minimum set of services to enable over the air programming of the device will
// still be brought up in pairing mode regardless of the settings below.
//

// Enable/Disable BLE during normal operation.
// Set '1' to enable.
#ifndef DEVICE_BLE_ENABLED
#define DEVICE_BLE_ENABLED                    1
#endif

// Enable/Disable BLE pairing mode mode at power up.
// Set '1' to enable.
#ifndef DEVICE_BLE_PAIRING_MODE
#define DEVICE_BLE_PAIRING_MODE               1
#endif

// Enable/Disable the use of private resolvable addresses.
// Set '1' to enable.
// n.b. This is known to be a feature that suffers compatibility issues with many BLE central devices.
#ifndef DEVICE_BLE_PRIVATE_ADDRESSES
#define DEVICE_BLE_PRIVATE_ADDRESSES          0
#endif

// Convenience option to enable / disable BLE security entirely
// Open BLE links are not secure, but commonly used during the development of BLE services
// Set '1' to disable all secuity
#ifndef DEVICE_BLE_OPEN
#define DEVICE_BLE_OPEN                       0
#endif

// Configure for open BLE operation if so configured
#if (DEVICE_BLE_OPEN == 1)
#define DEVICE_BLE_SECURITY_LEVEL             SECURITY_MODE_ENCRYPTION_OPEN_LINK
#define DEVICE_BLE_WHITELIST                  0
#define DEVICE_BLE_ADVERTISING_TIMEOUT        0
#define DEVICE_BLE_DEFAULT_TX_POWER           6
#endif


// Define the default, global BLE security requirements for Device BLE services
// May be one of the following options (see mbed's SecurityManager class implementaiton detail)
// SECURITY_MODE_ENCRYPTION_OPEN_LINK:      No bonding, encryption, or whitelisting required.
// SECURITY_MODE_ENCRYPTION_NO_MITM:        Bonding, encyption and whitelisting but no passkey.
// SECURITY_MODE_ENCRYPTION_WITH_MITM:      Bonding, encrytion and whitelisting with passkey authentication.
//
#ifndef DEVICE_BLE_SECURITY_LEVEL
#define DEVICE_BLE_SECURITY_LEVEL             SECURITY_MODE_ENCRYPTION_WITH_MITM
#endif

// Enable/Disable the use of BLE whitelisting.
// If enabled, the codal device will only respond to connection requests from
// known, bonded devices.
#ifndef DEVICE_BLE_WHITELIST
#define DEVICE_BLE_WHITELIST                  1
#endif

// Define the period of time for which the BLE stack will advertise (seconds)
// Afer this period, advertising will cease. Set to '0' for no timeout (always advertise).
#ifndef DEVICE_BLE_ADVERTISING_TIMEOUT
#define DEVICE_BLE_ADVERTISING_TIMEOUT        0
#endif

// Defines default power level of the BLE radio transmitter.
// Valid values are in the range 0..7 inclusive, with 0 being the lowest power and 7 the highest power.
// Based on trials undertaken by the BBC, the radio is normally set to its lowest power level
// to best protect children's privacy.
#ifndef DEVICE_BLE_DEFAULT_TX_POWER
#define DEVICE_BLE_DEFAULT_TX_POWER           0
#endif

// Enable/Disable BLE Service: DeviceDFU
// This allows over the air programming during normal operation.
// Set '1' to enable.
#ifndef DEVICE_BLE_DFU_SERVICE
#define DEVICE_BLE_DFU_SERVICE                1
#endif

// Enable/Disable BLE Service: DeviceEventService
// This allows routing of events from the codal device message bus over BLE.
// Set '1' to enable.
#ifndef DEVICE_BLE_EVENT_SERVICE
#define DEVICE_BLE_EVENT_SERVICE              1
#endif

// Enable/Disable BLE Service: DeviceDeviceInformationService
// This enables the standard BLE device information service.
// Set '1' to enable.
#ifndef DEVICE_BLE_DEVICE_INFORMATION_SERVICE
#define DEVICE_BLE_DEVICE_INFORMATION_SERVICE 1
#endif

//
// Accelerometer options
//

// Enable this to read 10 bits of data from the acclerometer.
// Otherwise, 8 bits are used.
// Set '1' to enable.
#ifndef USE_ACCEL_LSB
#define USE_ACCEL_LSB                           0
#endif

//
// Display options
//

// Selects the matrix configuration for the display driver.
// Known, acceptable options are:
//
#define MICROBUG_REFERENCE_DEVICE               1
#define DEVICE_3X9                            2
#define DEVICE_SB1                            3
#define DEVICE_SB2                            4

#ifndef DEVICE_DISPLAY_TYPE
#define DEVICE_DISPLAY_TYPE                   DEVICE_SB2
#endif

// Selects the minimum permissable brightness level for the device
// in the region of 0 (off) to 255 (full brightness)
#ifndef DEVICE_DISPLAY_MINIMUM_BRIGHTNESS
#define DEVICE_DISPLAY_MINIMUM_BRIGHTNESS     1
#endif

// Selects the maximum permissable brightness level for the device
// in the region of 0 (off) to 255 (full brightness)
#ifndef DEVICE_DISPLAY_MAXIMUM_BRIGHTNESS
#define DEVICE_DISPLAY_MAXIMUM_BRIGHTNESS     255
#endif

// Selects the default brightness for the display
// in the region of zero (off) to 255 (full brightness)
#ifndef DEVICE_DISPLAY_DEFAULT_BRIGHTNESS
#define DEVICE_DISPLAY_DEFAULT_BRIGHTNESS     DEVICE_DISPLAY_MAXIMUM_BRIGHTNESS
#endif

// Selects the default scroll speed for the display.
// The time taken to move a single pixel (ms).
#ifndef DEVICE_DEFAULT_SCROLL_SPEED
#define DEVICE_DEFAULT_SCROLL_SPEED           120
#endif

// Selects the number of pixels a scroll will move in each quantum.
#ifndef DEVICE_DEFAULT_SCROLL_STRIDE
#define DEVICE_DEFAULT_SCROLL_STRIDE          -1
#endif

// Selects the time each character will be shown on the display during print operations.
// The time each character is shown on the screen  (ms).
#ifndef DEVICE_DEFAULT_PRINT_SPEED
#define DEVICE_DEFAULT_PRINT_SPEED            400
#endif

//Configures the default serial mode used by serial read and send calls.
#ifndef DEVICE_DEFAULT_SERIAL_MODE
#define DEVICE_DEFAULT_SERIAL_MODE            SYNC_SLEEP
#endif


//
// I/O Options
//
#ifndef DEVICE_COMPONENT_COUNT
#define DEVICE_COMPONENT_COUNT               30
#endif
//
// Define the default mode in which the digital input pins are configured.
// valid options are PullDown, PullUp and PullNone.
//
#ifndef DEVICE_DEFAULT_PULLMODE
#define DEVICE_DEFAULT_PULLMODE                PullNone
#endif

//
// Panic options
//

// Enable this to invoke a panic on out of memory conditions.
// Set '1' to enable.
#ifndef DEVICE_PANIC_HEAP_FULL
#define DEVICE_PANIC_HEAP_FULL                1
#endif

//
// Debug options
//
#ifndef DEVICE_DMESG
#define DEVICE_DMESG                          0
#endif

// When non-zero internal debug messages (DMESG() macro) go to a in-memory buffer of this size (in bytes).
// It can be inspected from GDB (with 'print codalLogStore'), or accessed by the application.
// Typical size range between 512 and 4096. Set to 0 to disable.
#ifndef DEVICE_DMESG_BUFFER_SIZE
#define DEVICE_DMESG_BUFFER_SIZE              1024
#endif

// Enable this to route debug messages through the USB serial interface.
// n.b. This also disables the user serial port 'uBit.serial'.
// Set '1' to enable.
#ifndef DEVICE_DBG
#define DEVICE_DBG                            1
#endif

// Enable this to receive diagnostic messages from the heap allocator via the USB serial interface.
// n.b. This requires DEVICE_DBG to be defined.
// Set '1' to enable.
#ifndef DEVICE_HEAP_DBG
#define DEVICE_HEAP_DBG                       0
#endif

// Versioning options.
// We use semantic versioning (http://semver.org/) to identify differnet versions of the codal device runtime.
// Where possible we use yotta (an ARM mbed build tool) to help us track versions.
// if this isn't available, it can be defined manually as a configuration option.
//
#ifndef DEVICE_DAL_VERSION
#define DEVICE_DAL_VERSION                    "unknown"
#endif

#ifndef DEVICE_USB
#define DEVICE_USB                            1
#endif

//
// Helper macro used by the codal device runtime to determine if a boolean configuration option is set.
//
#define CONFIG_ENABLED(X) (X == 1)
#define CONFIG_DISABLED(X) (X != 1)

#if CONFIG_ENABLED(DEVICE_DBG)
extern RawSerial* SERIAL_DEBUG;
#endif

#endif
