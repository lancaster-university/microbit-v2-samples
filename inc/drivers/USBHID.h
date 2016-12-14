#include "CodalUSB.h"

#define USB_HID_ENDPOINTS       2

typedef struct {
    uint8_t len;
    uint8_t type; // 0x21
    uint16_t hidBCD; // 0x100
    uint8_t countryCode;
    uint8_t numDesc;
    uint8_t reportDescType; // 0x22
    uint16_t sizeOfReport;
} HIDReportDescriptor;

typedef struct
{
    InterfaceDescriptor iface;
    HIDReportDescriptor rdesc;
    EndpointDescriptor  in;
    EndpointDescriptor  out;
} HIDDescriptor;

class USBHID : public CodalUSBInterface
{
    public:
    USBHID();

    UsbEndpointIn *in;
    UsbEndpointOut *out;

    virtual int classRequest(UsbEndpointIn &ctrl, USBSetup& setup);
    virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup& setup);
    virtual int endpointRequest();
    virtual uint8_t getEndpointCount();
    virtual void initEndpoints(uint8_t firstEndpointIdx);
    virtual uint16_t getDescriptorSize();
    virtual void getDescriptor(uint8_t *dst);
};
