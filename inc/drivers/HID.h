#ifndef DEVICE_HID_H
#define DEVICE_HID_H

#include "CodalUSB.h"

#if CONFIG_ENABLED(DEVICE_USB)

#define HID_REQUEST_GET_REPORT 0x01
#define HID_REQUEST_GET_IDLE 0x02
#define HID_REQUEST_GET_PROTOCOL 0x03
#define HID_REQUEST_SET_REPORT 0x09
#define HID_REQUEST_SET_IDLE 0x0A
#define HID_REQUEST_SET_PROTOCOL 0x0B


typedef struct {
    uint8_t len;
    uint8_t type; // 0x21
    uint16_t hidBCD; // 0x100
    uint8_t countryCode;
    uint8_t numDesc;
    uint8_t reportDescType; // 0x22
    uint16_t sizeOfReport;
} __attribute__((packed)) HIDReportDescriptor;

class USBHID : public CodalUSBInterface
{
    public:
    USBHID();

    virtual int classRequest(UsbEndpointIn &ctrl, USBSetup& setup);
    virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup& setup);
    virtual int endpointRequest();
    virtual const InterfaceInfo *getInterfaceInfo();
};

#endif

#endif
