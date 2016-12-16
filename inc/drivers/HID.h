#include "CodalUSB.h"

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
