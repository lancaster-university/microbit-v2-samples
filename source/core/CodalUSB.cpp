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

extern const DeviceDescriptor device_descriptor;
extern const char *string_descriptors[];

LIST_HEAD(usb_list);

struct InterfaceList
{
    CodalUSBInterface *interface;
    struct list_head list;
};

static const ConfigDescriptor static_config = {9, 2, 0, 0, 1, 0, USB_CONFIG_BUS_POWERED, 250};

int CodalUSB::sendConfig()
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;
    int numInterfaces = 0;
    int clen = sizeof(ConfigDescriptor);

    // calculate the total size of our interfaces.
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);

        clen += tmp->interface->getDescriptorSize();
        numInterfaces++;
    }

    uint8_t *buf = new uint8_t[clen];
    memcpy(buf, &static_config, sizeof(ConfigDescriptor));
    ((ConfigDescriptor *)buf)->clen = clen;
    ((ConfigDescriptor *)buf)->numInterfaces = numInterfaces;
    clen = sizeof(ConfigDescriptor);

    // send our descriptors
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        tmp->interface->getDescriptor(buf + clen);
        clen += tmp->interface->getDescriptorSize();
    }

    usb_assert(clen == ((ConfigDescriptor *)buf)->clen);

    send(buf, clen);

    delete buf;

    return DEVICE_OK;
}

// languageID - United States
const uint8_t string0[] = {4, 3, 9, 4};

int CodalUSB::sendDescriptors(USBSetup &setup)
{
    uint8_t type = setup.wValueH;

    if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        return sendConfig();

    if (type == USB_DEVICE_DESCRIPTOR_TYPE)
        return send(&device_descriptor, sizeof(DeviceDescriptor));

    else if (type == USB_STRING_DESCRIPTOR_TYPE)
    {
        // check if we exceed our bounds.
        if (setup.wValueL > STRING_DESCRIPTOR_COUNT - 1)
            return DEVICE_NOT_SUPPORTED;

        if (setup.wValueL == 0)
            return send(string0, sizeof(string0));

        StringDescriptor desc;

        const char *str = string_descriptors[setup.wValueL];
        if (!str)
            return DEVICE_NOT_SUPPORTED;

        desc.type = 3;
        desc.len = strlen(str) * 2 + 2;

        usb_assert(desc.len <= sizeof(desc));

        int i = 0;
        while (*str)
            desc.data[i++] = *str++;

        // send the string descriptor the host asked for.
        return send(&desc, desc.len);
    }

    return DEVICE_OK;
}

CodalUSB::CodalUSB()
{
    usbInstance = this;
    endpointsUsed = 1; // CTRL endpoint
    ctrlIn = NULL;
    ctrlOut = NULL;
}

CodalUSB *CodalUSB::getInstance()
{
    if (usbInstance == NULL)
        usbInstance = new CodalUSB;

    return usbInstance;
}

int CodalUSB::configureEndpoints()
{
    uint8_t endpointCount = 1;

    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    uint8_t iEpCount = 0;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        iEpCount = tmp->interface->getEndpointCount();
        tmp->interface->initEndpoints(endpointCount);

        endpointCount += iEpCount;
    }

    usb_assert(endpointsUsed == endpointCount);

    return DEVICE_OK;
}

int CodalUSB::add(CodalUSBInterface &interface)
{
    usb_assert(ctrlIn == NULL);

    uint8_t epsConsumed = interface.getEndpointCount();

    if (endpointsUsed + epsConsumed > DEVICE_USB_ENDPOINTS)
        return DEVICE_NO_RESOURCES;

    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list) tmp = list_entry(iter, InterfaceList, list);

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

int CodalUSB::classRequest(USBSetup &setup)
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        int res = tmp->interface->classRequest(*ctrlIn, setup);
        if (res == DEVICE_OK)
            return DEVICE_OK;
    }

    return DEVICE_NOT_SUPPORTED;
}

#define sendzlp() send(&usb_status, 0)
#define stall ctrlIn->stall

int CodalUSB::ctrlRequest()
{
    if (!usb_recieved_setup())
        return 0;

    USBSetup setup;
    int len = ctrlOut->read(&setup, sizeof(setup));
    usb_assert(len == sizeof(setup));
    usb_clear_setup();

    int status = DEVICE_OK;

    // Standard Requests
    uint16_t wValue = (setup.wValueH << 8) | setup.wValueL;
    uint8_t request_type = setup.bmRequestType;
    uint16_t wStatus = 0;

    ctrlIn->wLength = setup.wLength;

    if ((request_type & TYPE) == REQUEST_TYPE_STANDARD)
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
            if (REQUEST_DEVICE == (request_type & REQUEST_RECIPIENT))
            {
                configureEndpoints();
                usb_initialised = setup.wValueL;
            }
            else
                status = DEVICE_NOT_SUPPORTED;
            break;
        }
    }
    else
    {
        status = classRequest(setup);
    }

    if (status < 0)
        stall();

    return 0;
}

int CodalUSB::interruptHandler()
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        tmp->interface->endpointRequest();
    }

    return DEVICE_NOT_SUPPORTED;
}

int CodalUSB::start()
{
    if (DEVICE_USB_ENDPOINTS == 0)
        return DEVICE_NOT_SUPPORTED;

    if (ctrlIn != NULL)
        return DEVICE_OK;

    usb_configure(endpointsUsed);

    ctrlIn = new UsbEndpointIn(0, USB_EP_TYPE_CONTROL);
    ctrlOut = new UsbEndpointOut(0, USB_EP_TYPE_CONTROL);

    return DEVICE_OK;
}
#endif
