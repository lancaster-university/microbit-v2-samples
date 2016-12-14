#include "USBHID.h"

static const char hidDescriptor[] = {
    0x06, 0x00, 0xFF, // usage page vendor #0
    0x09, 0x01,       // usage 1
    0xA1, 0x01,       // collection - application
    0x15, 0x00,       // logical min 0
    0x26, 0xFF, 0x00, // logical max 255
    0x75, 8,          // report size 8
    0x95, 64,         // report count 64
    0x09, 0x01,       // usage 1
    0x81, 0x02,       // input: data, variable, absolute
    0x95, 64,         // report count 64
    0x09, 0x01,       // usage 1
    0x91, 0x02,       // output: data, variable, absolute
    0x95, 1,          // report count 1
    0x09, 0x01,       // usage 1
    0xB1, 0x02,       // feature: data, variable, absolute
    0xC0,             // end
};

static const HIDDescriptor static_hid_descriptor = {
    {
        9,                 /// descriptor size in bytes
        4,                 /// descriptor type - interface
        0,                 /// interface number
        0,                 /// alternate setting number
        USB_HID_ENDPOINTS, /// number of endpoints
        0x03,              /// class code - HID
        0,                 /// subclass code
        0,                 /// protocol code
        0,                 /// interface string index
    },
    {
        9,
        0x21,                  // HID
        0x100,                 // hidbcd 1.00
        0x00,                  // country code
        0x01,                  // num desc
        0x22,                  // report desc type
        sizeof(hidDescriptor), // size of 0x22
    },
    EP_DESC2(USB_EP_TYPE_INTERRUPT, 1),
};

USBHID::USBHID() : CodalUSBInterface()
{
}

int USBHID::stdRequest(UsbEndpointIn &ctrl, USBSetup &setup)
{
    if (setup.bRequest == GET_DESCRIPTOR)
    {
        if (setup.wValueH == 0x21)
        {
            InterfaceDescriptor tmp = static_hid_descriptor.iface;
            tmp.number = interfaceIdx;
            return ctrl.write(&tmp, sizeof(tmp));
        }
        else if (setup.wValueH == 0x22)
        {
            return ctrl.write(hidDescriptor, sizeof(hidDescriptor));
        }
    }
    return DEVICE_NOT_SUPPORTED;
}

int USBHID::endpointRequest()
{
    uint8_t buf[64];
    int len = out->read(buf, sizeof(buf));
    if (len <= 0)
        return len;

    // this should echo-back serial
    return in->write(buf, sizeof(buf));
}

uint8_t USBHID::getEndpointCount()
{
    // in/out use the same interrupt endpoint
    return 1;
}

void USBHID::initEndpoints(uint8_t firstEndpointIdx)
{
    in = new UsbEndpointIn(firstEndpointIdx, USB_EP_TYPE_INTERRUPT);
    out = new UsbEndpointOut(firstEndpointIdx, USB_EP_TYPE_INTERRUPT);
}

uint16_t USBHID::getDescriptorSize()
{
    return sizeof(static_hid_descriptor);
}

void USBHID::getDescriptor(uint8_t *dst)
{
    // dst might be not aligned, use a local
    HIDDescriptor tmp = static_hid_descriptor;
    tmp.iface.number = interfaceIdx;
    tmp.in.addr = 0x80 | in->ep;
    tmp.out.addr = out->ep;
    memcpy(dst, &tmp, sizeof(tmp));
}

#define HID_REQUEST_GET_REPORT 0x01
#define HID_REQUEST_GET_IDLE 0x02
#define HID_REQUEST_GET_PROTOCOL 0x03
#define HID_REQUEST_SET_REPORT 0x09
#define HID_REQUEST_SET_IDLE 0x0A
#define HID_REQUEST_SET_PROTOCOL 0x0B

int USBHID::classRequest(UsbEndpointIn &ctrl, USBSetup &setup)
{
    uint8_t buf[64] = {0};

    switch (setup.bRequest)
    {
    case HID_REQUEST_GET_PROTOCOL:
    case HID_REQUEST_GET_IDLE:
    case HID_REQUEST_GET_REPORT:
        return ctrl.write(buf, 64);

    case HID_REQUEST_SET_IDLE:
    case HID_REQUEST_SET_REPORT:
    case HID_REQUEST_SET_PROTOCOL:
        return ctrl.write(buf, 0);
    }

    return DEVICE_NOT_SUPPORTED;
}
