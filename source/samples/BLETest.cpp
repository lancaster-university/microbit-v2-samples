/*
The MIT License (MIT)

Copyright (c) 2021 Lancaster University.

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

#include "MicroBit.h"
#include "Tests.h"

#if CONFIG_ENABLED(DEVICE_BLE)

extern MicroBit uBit;
MicroBitUARTService *uart;

// we use events abd the 'connected' variable to keep track of the status of the Bluetooth connection
static void onConnected(MicroBitEvent)
{
    uBit.display.print("C");
}

static void onDisconnected(MicroBitEvent)
{
    uBit.display.print("D");
}

static void onDelim(MicroBitEvent)
{
    ManagedString r = uart->readUntil("\r\n");
    uart->send(r);
}

void ble_test()
{
    // Configuration Tips
    //
    // An example codal.json is provided in the root directory: codal.ble.json
    // Rename codal.ble.json to codal.json to use this BLE sample
    //
    // codal.json contains various Bluetooth related properties some of which are explained here:
    //
    // "DEVICE_BLE": 1                     Determines whether BLE is enabled 
    // "MICROBIT_BLE_ENABLED" : 1          Determines whether BLE is enabled
    // "MICROBIT_BLE_PAIRING_MODE": 1      Determines whether Pairing Mode is enabled
    // "MICROBIT_BLE_DFU_SERVICE": 1       Determines whether the Nordic DFU Service is enabled
    // "MICROBIT_BLE_DEVICE_INFORMATION_SERVICE": 1 Determines whether the DIS is enabled
    // "MICROBIT_BLE_EVENT_SERVICE" : 1,   Determines whether the Event Service is enabled
    // "MICROBIT_BLE_PARTIAL_FLASHING" : 0 Determines whether Partial Flashing is enabled (Needs MakeCode/Python)
    // "MICROBIT_BLE_SECURITY_LEVEL": "SECURITY_MODE_ENCRYPTION_WITH_MITM" Determines security mode
    //
    // Options for MICROBIT_BLE_SECURITY_LEVEL are:
    // SECURITY_MODE_ENCRYPTION_WITH_MITM, enables pairing with a passcode
    // SECURITY_MODE_ENCRYPTION_NO_MITM, enables pairing without a passcode
    // SECURITY_MODE_ENCRYPTION_OPEN_LINK, pairing is no required
    //

    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, onConnected);
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, onDisconnected);
    
    uBit.messageBus.listen(MICROBIT_ID_BLE_UART, MICROBIT_UART_S_EVT_DELIM_MATCH, onDelim);

    new MicroBitAccelerometerService(*uBit.ble, uBit.accelerometer);
    new MicroBitButtonService(*uBit.ble);
    new MicroBitIOPinService(*uBit.ble, uBit.io);
    new MicroBitLEDService(*uBit.ble, uBit.display);
    new MicroBitMagnetometerService(*uBit.ble, uBit.compass);
    new MicroBitTemperatureService(*uBit.ble, uBit.thermometer);

    uart = new MicroBitUARTService(*uBit.ble, 32, 32);
    uart->eventOn("\r\n");

    // A cunning code to indicate during start-up the particular Bluetooth configuration in the build
    //
    // SERVICE CODES
    // A: Accelerometer Service
    // B: Button Service
    // D: Device Information Service
    // E: Event Service
    // F: DFU Service
    // I: IO Pin Service
    // L: LED Service
    // M: Magnetometer Service
    // T: Temperature Service
    // U: UART Service
    //

    // P: PASSKEY
    // J: Just Works
    // N: No Pairing Required

    // Services/Pairing Config/Power Level
    uBit.display.scroll("BLE ABDILMTU/P");

    if ( !uBit.compass.isCalibrated())
        uBit.compass.calibrate();

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}

#endif