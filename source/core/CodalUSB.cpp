#include "CodalUSB.h"

#if CONFIG_ENABLED(DEVICE_USB)

#include "ErrorNo.h"
#include "list.h"

#define send(p, l) ctrlIn->write(p, l)

CodalUSB *CodalUSB::usbInstance = NULL;

static uint8_t usb_initialised = 0;
// usb_20.pdf
static uint8_t usb_status = 0;
// static uint8_t usb_suspended = 0; // copy of UDINT to check SUSPI and WAKEUPI bits
static uint8_t usb_configured = 0;

LIST_HEAD(usb_list);

struct InterfaceList
{
    CodalUSBInterface *interface;
    struct list_head list;
};

static const ConfigDescriptor static_config = {9, 2, 0, 0, 1, 0, USB_CONFIG_BUS_POWERED, 250};

static const DeviceDescriptor default_device_desc = {
    0x12,            // bLength
    0x01,            // bDescriptorType
    0x0200,          // bcdUSBL
    0xEF,            // bDeviceClass:    Misc
    0x02,            // bDeviceSubclass:
    0x01,            // bDeviceProtocol:
    0x40,            // bMaxPacketSize0
    USB_DEFAULT_VID, //
    USB_DEFAULT_PID, //
    0x4202,          // bcdDevice - leave unchanged for the HF2 to work
    0x01,            // iManufacturer
    0x02,            // iProduct
    0x03,            // SerialNumber
    0x01             // bNumConfigs
};

static const char *default_strings[] = {
    "CoDAL Devices", "Generic CoDAL device", "4242",
};

CodalUSB::CodalUSB()
{
    usbInstance = this;
    endpointsUsed = 1; // CTRL endpoint
    ctrlIn = NULL;
    ctrlOut = NULL;
    numStringDescriptors = sizeof(default_strings) / sizeof(default_strings[0]);
    stringDescriptors = default_strings;
    deviceDescriptor = &default_device_desc;
}

void CodalUSBInterface::fillInterfaceInfo(InterfaceDescriptor *descp)
{
    const InterfaceInfo *info = this->getInterfaceInfo();
    InterfaceDescriptor desc = {
        sizeof(InterfaceDescriptor),
        4, // type
        this->interfaceIdx,
        info->iface.alternate,
        info->iface.numEndpoints,
        info->iface.interfaceClass,
        info->iface.interfaceSubClass,
        info->iface.protocol,
        info->iface.iInterfaceString,
    };
    *descp = desc;
}

int CodalUSB::sendConfig()
{
    InterfaceList *tmp = NULL;
    const InterfaceInfo *info = NULL;
    struct list_head *iter, *q = NULL;
    int numInterfaces = 0;
    int clen = sizeof(ConfigDescriptor);

    // calculate the total size of our interfaces.
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);

        info = tmp->interface->getInterfaceInfo();
        clen += sizeof(InterfaceDescriptor) +
                info->iface.numEndpoints * sizeof(EndpointDescriptor) +
                info->supplementalDescriptorSize;
        numInterfaces++;
    }

    uint8_t *buf = new uint8_t[clen];
    memcpy(buf, &static_config, sizeof(ConfigDescriptor));
    ((ConfigDescriptor *)buf)->clen = clen;
    ((ConfigDescriptor *)buf)->numInterfaces = numInterfaces;
    clen = sizeof(ConfigDescriptor);

#define ADD_DESC(desc)                                                                             \
    memcpy(buf + clen, &desc, sizeof(desc));                                                       \
    clen += sizeof(desc)

    // send our descriptors
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        info = tmp->interface->getInterfaceInfo();
        InterfaceDescriptor desc;
        tmp->interface->fillInterfaceInfo(&desc);
        ADD_DESC(desc);

        if (info->supplementalDescriptorSize)
        {
            memcpy(buf + clen, info->supplementalDescriptor, info->supplementalDescriptorSize);
            clen += info->supplementalDescriptorSize;
        }

        EndpointDescriptor epdescIn = {
            sizeof(EndpointDescriptor),
            5, // type
            (uint8_t)(0x80 | tmp->interface->in->ep),
            info->epIn.attr,
            USB_MAX_PKT_SIZE,
            info->epIn.interval,
        };
        ADD_DESC(epdescIn);

        if (info->iface.numEndpoints == 1)
        {
            // OK
        }
        else if (info->iface.numEndpoints == 2)
        {
            EndpointDescriptor epdescOut = {
                sizeof(EndpointDescriptor),
                5, // type
                tmp->interface->out->ep,
                info->epIn.attr,
                USB_MAX_PKT_SIZE,
                info->epIn.interval,
            };
            ADD_DESC(epdescOut);
        }
        else
        {
            usb_assert(0);
        }
    }

    usb_assert(clen == ((ConfigDescriptor *)buf)->clen);

    send(buf, clen);

    delete buf;

    return DEVICE_OK;
}

// languageID - United States
static const uint8_t string0[] = {4, 3, 9, 4};

int CodalUSB::sendDescriptors(USBSetup &setup)
{
    uint8_t type = setup.wValueH;

    if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        return sendConfig();

    if (type == USB_DEVICE_DESCRIPTOR_TYPE)
        return send(deviceDescriptor, sizeof(DeviceDescriptor));

    else if (type == USB_STRING_DESCRIPTOR_TYPE)
    {
        // check if we exceed our bounds.
        if (setup.wValueL > numStringDescriptors)
            return DEVICE_NOT_SUPPORTED;

        if (setup.wValueL == 0)
            return send(string0, sizeof(string0));

        StringDescriptor desc;

        const char *str = stringDescriptors[setup.wValueL - 1];
        if (!str)
            return DEVICE_NOT_SUPPORTED;

        desc.type = 3;
        uint32_t len = strlen(str) * 2 + 2;
        desc.len = len;

        usb_assert(len <= sizeof(desc));

        int i = 0;
        while (*str)
            desc.data[i++] = *str++;

        // send the string descriptor the host asked for.
        return send(&desc, desc.len);
    }
    else
    {
        return interfaceRequest(setup, false);
    }

    return DEVICE_NOT_SUPPORTED;
}

CodalUSB *CodalUSB::getInstance()
{
    if (usbInstance == NULL)
        usbInstance = new CodalUSB;

    return usbInstance;
}

int CodalUSB::add(CodalUSBInterface &interface)
{
    usb_assert(!usb_configured);

    uint8_t epsConsumed = interface.getInterfaceInfo()->allocateEndpoints;

    if (endpointsUsed + epsConsumed > DEVICE_USB_ENDPOINTS)
        return DEVICE_NO_RESOURCES;

    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    interface.interfaceIdx = 0;

    list_for_each_safe(iter, q, &usb_list)
    {
        interface.interfaceIdx++;
        tmp = list_entry(iter, InterfaceList, list);
    }

    tmp = new InterfaceList;

    tmp->interface = &interface;

    list_add(&tmp->list, iter);

    endpointsUsed += epsConsumed;

    return DEVICE_OK;
}

int CodalUSB::isInitialised()
{
    return usb_initialised > 0;
}

int CodalUSB::interfaceRequest(USBSetup &setup, bool isClass)
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    int ifaceIdx = -1;
    int epIdx = -1;

    if ((setup.bmRequestType & REQUEST_DESTINATION) == REQUEST_INTERFACE)
        ifaceIdx = setup.wIndex & 0xff;
    else if ((setup.bmRequestType & REQUEST_DESTINATION) == REQUEST_ENDPOINT)
        epIdx = setup.wIndex & 0xff;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        CodalUSBInterface *iface = tmp->interface;
        if (iface->interfaceIdx == ifaceIdx ||
            ((iface->in && iface->in->ep == epIdx) || (iface->out && iface->out->ep == epIdx)))
        {
            int res = isClass ? tmp->interface->classRequest(*ctrlIn, setup)
                              : tmp->interface->stdRequest(*ctrlIn, setup);
            if (res == DEVICE_OK)
                return DEVICE_OK;
        }
    }

    return DEVICE_NOT_SUPPORTED;
}

#define sendzlp() send(&usb_status, 0)
#define stall ctrlIn->stall

void CodalUSB::setupRequest(USBSetup &setup)
{
    DMESG("SETUP Req=%x type=%x val=%x:%x idx=%x len=%d", setup.bRequest, setup.bmRequestType,
          setup.wValueH, setup.wValueL, setup.wIndex, setup.wLength);

    int status = DEVICE_OK;

    // Standard Requests
    uint16_t wValue = (setup.wValueH << 8) | setup.wValueL;
    uint8_t request_type = setup.bmRequestType;
    uint16_t wStatus = 0;

    ctrlIn->wLength = setup.wLength;

    if ((request_type & REQUEST_TYPE) == REQUEST_STANDARD)
    {
        switch (setup.bRequest)
        {
        case GET_STATUS:
            if (request_type == (REQUEST_DEVICETOHOST | REQUEST_STANDARD | REQUEST_DEVICE))
            {
                wStatus = usb_status;
            }
            send(&wStatus, sizeof(wStatus));
            break;

        case CLEAR_FEATURE:
            if ((request_type == (REQUEST_HOSTTODEVICE | REQUEST_STANDARD | REQUEST_DEVICE)) &&
                (wValue == DEVICE_REMOTE_WAKEUP))
                usb_status &= ~FEATURE_REMOTE_WAKEUP_ENABLED;
            sendzlp();
            break;
        case SET_FEATURE:
            if ((request_type == (REQUEST_HOSTTODEVICE | REQUEST_STANDARD | REQUEST_DEVICE)) &&
                (wValue == DEVICE_REMOTE_WAKEUP))
                usb_status |= FEATURE_REMOTE_WAKEUP_ENABLED;
            sendzlp();
            break;
        case SET_ADDRESS:
            sendzlp();
            usb_set_address(wValue);
            break;
        case GET_DESCRIPTOR:
            status = sendDescriptors(setup);
            break;
        case SET_DESCRIPTOR:
            stall();
            break;
        case GET_CONFIGURATION:
            wStatus = 1;
            send(&wStatus, 1);
            break;

        case SET_CONFIGURATION:
            if (REQUEST_DEVICE == (request_type & REQUEST_DESTINATION))
            {
                usb_initialised = setup.wValueL;
                sendzlp();
            }
            else
                status = DEVICE_NOT_SUPPORTED;
            break;
        }
    }
    else
    {
        status = interfaceRequest(setup, true);
    }

    if (status < 0)
        stall();

    // sending response clears this - make sure we did
    usb_assert(ctrlIn->wLength == 0);
}

void CodalUSB::interruptHandler()
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        tmp->interface->endpointRequest();
    }
}

void CodalUSB::initEndpoints()
{
    uint8_t endpointCount = 1;

    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    ctrlIn = new UsbEndpointIn(0, USB_EP_TYPE_CONTROL);
    ctrlOut = new UsbEndpointOut(0, USB_EP_TYPE_CONTROL);

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);

        const InterfaceInfo *info = tmp->interface->getInterfaceInfo();

        usb_assert(1 <= info->allocateEndpoints && info->allocateEndpoints <= 2);
        usb_assert(info->allocateEndpoints <= info->iface.numEndpoints &&
                   info->iface.numEndpoints <= 2);

        tmp->interface->in = new UsbEndpointIn(endpointCount, info->epIn.attr);
        if (info->iface.numEndpoints > 1)
        {
            tmp->interface->out =
                new UsbEndpointOut(endpointCount + (info->allocateEndpoints - 1), info->epIn.attr);
        }

        endpointCount += info->allocateEndpoints;
    }

    usb_assert(endpointsUsed == endpointCount);
}

int CodalUSB::start()
{
    DMESG("USB start");

    if (DEVICE_USB_ENDPOINTS == 0)
        return DEVICE_NOT_SUPPORTED;

    if (usb_configured)
        return DEVICE_OK;

    usb_configured = 1;

    usb_configure(endpointsUsed);

    return DEVICE_OK;
}

void usb_panic(int lineNumber)
{
    DMESG("USB assertion failed: line %d", lineNumber);
    device.panic(DEVICE_USB_ERROR);
}

#endif
