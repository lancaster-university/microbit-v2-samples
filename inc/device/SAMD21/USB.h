#ifndef DEVICE_USB_H
#define DEVICE_USB_H

#if CONFIG_ENABLED(DEVICE_USB)

#include "CodalDevice.h"
#include "CodalDmesg.h"
#include "samd21.h"

#define DEVICE_USB_ENDPOINTS USB_EPT_NUM
#define USB_MAX_PKT_SIZE 64

#define USB_DEFAULT_PID 0x2402 // Generic ASF HID device
#define USB_DEFAULT_VID 0x03EB // Atmel

class UsbEndpointIn
{
    uint8_t buf[USB_MAX_PKT_SIZE];

public:
    uint8_t ep;
    uint8_t flags;
    uint16_t wLength;
    int stall();
    int reset();
    int write(const void *buf, int length);

    UsbEndpointIn(uint8_t idx, uint8_t type, uint8_t size = USB_MAX_PKT_SIZE);
};

#define USB_EP_FLAG_NO_AUTO_ZLP 0x01

class UsbEndpointOut
{
    uint8_t buf[USB_MAX_PKT_SIZE];
    void startRead();

public:
    uint8_t ep;
    int stall();
    int reset();

    int read(void *buf, int maxlength); // up to packet size
    // int readBlocking(const void *buf, int length);

    UsbEndpointOut(uint8_t idx, uint8_t type, uint8_t size = USB_MAX_PKT_SIZE);
};

void usb_configure(uint8_t numEndpoints);
void usb_set_address(uint16_t wValue);

#endif
#endif
