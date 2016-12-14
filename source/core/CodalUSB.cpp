#include "CodalUSB.h"

#if CONFIG_ENABLED(DEVICE_USB)

#include "ErrorNo.h"
#include "list.h"

#define send(p, l) ctrlIn->write(p, l)

CodalUSB *CodalUSB::usbInstance = NULL;

#if 0
static volatile uint8_t usb_initialised = 0;
// usb_20.pdf
static volatile uint8_t usb_status = 0;
static volatile uint8_t usb_suspended = 0; // copy of UDINT to check SUSPI and WAKEUPI bits
#endif

extern const DeviceDescriptor device_descriptor;
extern const StringDescriptor string_descriptors[];

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
    ((ConfigDescriptor*)buf)->clen = clen;
    ((ConfigDescriptor*)buf)->numInterfaces = numInterfaces;
    clen = sizeof(ConfigDescriptor);

    // send our descriptors
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        tmp->interface->getDescriptor(buf + clen);
        clen += tmp->interface->getDescriptorSize();
    }

    usb_assert(clen == ((ConfigDescriptor*)buf)->clen);

    send(buf, clen);

    delete buf;

    return DEVICE_OK;
}

int CodalUSB::sendDescriptors(USBSetup &setup)
{
    uint8_t type = setup.wValueH;

    ctrlIn->wLength = setup.wLength;

    if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        return sendConfig();

    if (type == USB_DEVICE_DESCRIPTOR_TYPE)
        return send(&device_descriptor, sizeof(DeviceDescriptor));

    else if (type == USB_STRING_DESCRIPTOR_TYPE)
    {
        // check if we exceed our bounds.
        if (setup.wValueL > STRING_DESCRIPTOR_COUNT - 1)
            return DEVICE_NOT_SUPPORTED;

        // send the string descriptor the host asked for.
        return send(&string_descriptors[setup.wValueL],
                             sizeof(string_descriptors[setup.wValueL]));
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

    ctrlIn = new UsbEndpointIn(0, USB_EP_TYPE_CONTROL);
    ctrlOut = new UsbEndpointOut(0, USB_EP_TYPE_CONTROL);

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
    usb_assert(!isInitialised());

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
    return ctrlIn != NULL;
}

int CodalUSB::classRequest(USBSetup &setup)
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        tmp->interface->classRequest(setup);

        // if( == DEVICE_OK)
        //    return DEVICE_OK;
    }

    return DEVICE_OK;
}

int CodalUSB::endpointRequest(uint8_t endpoint)
{
    InterfaceList *tmp = NULL;
    struct list_head *iter, *q = NULL;

    uint8_t endpointCount = 1;
    uint8_t iEpCount = 0;
    uint8_t i = 0;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);

        iEpCount = tmp->interface->getEndpointCount();

        for (i = 0; i < iEpCount; i++)
        {
            if (endpointCount == endpoint)
            {
                tmp->interface->endpointRequest(endpoint);
                return DEVICE_OK;
            }

            endpointCount++;
        }
    }

    return DEVICE_NOT_SUPPORTED;
}

int CodalUSB::start()
{
    if (DEVICE_USB_ENDPOINTS == 0)
        return DEVICE_NOT_SUPPORTED;

    if (isInitialised())
        return DEVICE_OK;

    usb_configure(endpointsUsed);
    configureEndpoints();

    return DEVICE_OK;
}
#endif
