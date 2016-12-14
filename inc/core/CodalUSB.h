#ifndef CODAL_USB_H
#define CODAL_USB_H

#include "DeviceConfig.h"

#if CONFIG_ENABLED(DEVICE_USB)

#include <stdint.h>
#include "ErrorNo.h"
#include "USB.h"

#define GET_STATUS    0
#define CLEAR_FEATURE    1
//Reserved for future use    2
#define SET_FEATURE    3
//Reserved for future use    4
#define SET_ADDRESS    5
#define GET_DESCRIPTOR    6
#define SET_DESCRIPTOR    7
#define GET_CONFIGURATION    8
#define SET_CONFIGURATION    9
#define GET_INTERFACE    10
#define SET_INTERFACE    11
#define SYNCH_FRAME    12

#define DIRECTION               (1 << 7)
#define DIRECTION_OUT           0
#define DIRECTION_IN            1

#define TYPE                    (3 << 5)
#define REQUEST_TYPE_STANDARD   0
#define REQUEST_TYPE_CLASS      1
#define REQUEST_TYPE_VENDOR     2

#define STRING_DESCRIPTOR_COUNT     4

#define USB_CONFIG_POWERED_MASK                0x40
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

#define USB_DEVICE_DESCRIPTOR_TYPE             1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      2
#define USB_STRING_DESCRIPTOR_TYPE             3
#define USB_INTERFACE_DESCRIPTOR_TYPE          4
#define USB_ENDPOINT_DESCRIPTOR_TYPE           5

#define REQUEST_HOSTTODEVICE    0x00
#define REQUEST_DEVICETOHOST    0x80
#define REQUEST_DIRECTION       0x80

#define REQUEST_STANDARD        0x00
#define REQUEST_CLASS           0x20
#define REQUEST_VENDOR          0x40
#define REQUEST_TYPE            0x60

#define REQUEST_DESTINATION     0x0F
#define REQUEST_DEVICE          0x00
#define REQUEST_INTERFACE       0x01
#define REQUEST_ENDPOINT        0x02
#define REQUEST_OTHER           0x03
#define REQUEST_RECIPIENT       0x03

#define REQUEST_GET_STATUS      0x00
#define REQUEST_CLEAR_FEATURE   0x01
#define REQUEST_SET_FEATURE     0x03
#define REQUEST_SYNCH_FRAME     0x12

#define DEVICE_REMOTE_WAKEUP             1
#define FEATURE_SELFPOWERED_ENABLED     (1 << 0)
#define FEATURE_REMOTE_WAKEUP_ENABLED   (1 << 1)

enum usb_ep_type {
	USB_EP_TYPE_CONTROL = 0x00,
	USB_EP_TYPE_ISOCHRONOUS = 0x01,
	USB_EP_TYPE_BULK = 0x02,
	USB_EP_TYPE_INTERRUPT = 0x03,
};

//    Device
typedef struct {
    uint8_t  len;                // 18
    uint8_t  dtype;            // 1 USB_DEVICE_DESCRIPTOR_TYPE
    uint16_t usbVersion;        // 0x200 or 0x210
    uint8_t  deviceClass;
    uint8_t  deviceSubClass;
    uint8_t  deviceProtocol;
    uint8_t  packetSize0;    // Packet 0
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t deviceVersion;    // 0x100
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} DeviceDescriptor;

//    Config
typedef struct {
    uint8_t    len;            // 9
    uint8_t    dtype;            // 2
    uint16_t clen;            // total length
    uint8_t    numInterfaces;
    uint8_t    config;
    uint8_t    iconfig;
    uint8_t    attributes;
    uint8_t    maxPower;
} ConfigDescriptor;

//    String

//    Interface
typedef struct
{
    uint8_t len;
    uint8_t dtype;
    uint8_t number;
    uint8_t alternate;
    uint8_t numEndpoints;
    uint8_t interfaceClass;
    uint8_t interfaceSubClass;
    uint8_t protocol;
    uint8_t iInterface;
} InterfaceDescriptor;

//    Endpoint
typedef struct
{
    uint8_t  len;
    uint8_t  dtype;
    uint8_t  addr;
    uint8_t  attr;
    uint16_t packetSize;
    uint8_t  interval;
} EndpointDescriptor;

typedef struct
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint8_t wValueL;
    uint8_t wValueH;
    uint16_t wIndex;
    uint16_t wLength;
} USBSetup;

typedef struct {
    uint8_t len;
    uint8_t type;
    uint8_t data[26];
} StringDescriptor;

#pragma GCC diagnostic ignored "-Wunused-parameter"

class CodalUSBInterface
{
    public:

    CodalUSBInterface() {}

    virtual int classRequest(USBSetup& setup) { return DEVICE_NOT_SUPPORTED; }
    virtual int endpointRequest() { return DEVICE_NOT_SUPPORTED; }

    virtual uint8_t getEndpointCount() { return 0; }

    virtual void initEndpoints(uint8_t firstEndpointIdx) { }

    virtual uint16_t getDescriptorSize() { return 0; }

    virtual void getDescriptor(uint8_t *dst) { }
};

class CodalUSB
{
    uint8_t endpointsUsed;

    int configureEndpoints();
    int sendConfig();
    int ctrlRequest();

    public:
    static CodalUSB *usbInstance;

    UsbEndpointIn *ctrlIn;
    UsbEndpointOut *ctrlOut;

    CodalUSB();

    int add(CodalUSBInterface& interface);

    int sendDescriptors(USBSetup& setup);

    int classRequest(USBSetup& setup);
    
    int endpointRequest();

    int isInitialised();

    CodalUSB* getInstance();

    int start();
};
#endif

#endif
