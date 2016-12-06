#include "CodalUSB.h"

#if CONFIG_ENABLED(DEVICE_USB)

#include "ErrorNo.h"
#include "list.h"

CodalUSB* CodalUSB::usbInstance = NULL;

static volatile uint8_t usb_initialised = 0;
// usb_20.pdf
static volatile uint8_t usb_status = 0;
static volatile uint8_t usb_suspended = 0; // copy of UDINT to check SUSPI and WAKEUPI bits

static int usb_packet_start;
static int usb_packet_end;

extern const DeviceDescriptor device_descriptor;
extern const StringDescriptor string_descriptors[];

LIST_HEAD(usb_list);

struct InterfaceList
{
    CodalUSBInterface* interface;
    struct list_head list;
};

static const ConfigDescriptor static_config = {
    9,
    2,
    0,
    0,
    1,
    0,
    USB_CONFIG_BUS_POWERED,
    250
};

void set_limits(int end)
{
    set_endpoint(0);
    usb_packet_start = 0;
    usb_packet_end = end;
}

void CodalUSB::configure()
{
    configure_usb();
}

bool CodalUSB::send(uint8_t data)
{
    if (usb_packet_start < usb_packet_end)
    {
        if (!wait_rx_tx_int())
            return false;

        send8(data);

        if (!((usb_packet_start + 1) & 0x3F))
            clear_tx_int();
    }

    usb_packet_start++;
    return true;
}

int CodalUSB::send(uint8_t* data, int len)
{
    uint8_t* end = (uint8_t*)data + len;

    while (data != end)
        if (!send(*data++))
            return end - data;

    return len;
}

uint8_t CodalUSB::read()
{
    return read8();
}

int CodalUSB::read(uint8_t* buf, int len)
{
    uint8_t* end = buf + len;

    while (buf != end)
        *buf++ = read();

    return len;
}

int CodalUSB::sendConfig(int maxLen)
{
    ConfigDescriptor config;
    memcpy(&config, &static_config, sizeof(ConfigDescriptor));

    InterfaceList* tmp = NULL;
    struct list_head *iter, *q = NULL;

    config.clen = sizeof(ConfigDescriptor);
    config.numInterfaces = 0;

    // calculate the total size of our interfaces.
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);

        config.clen += tmp->interface->getDescriptorSize();
        config.numInterfaces++;
    }

    set_limits(maxLen);
    send((uint8_t *)&config, sizeof(ConfigDescriptor));

    // send our descriptors
    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        send(tmp->interface->getDescriptor(), tmp->interface->getDescriptorSize());
    }

    return DEVICE_OK;
}

int CodalUSB::sendDescriptors(USBSetup& setup)
{
    uint8_t type = setup.wValueH;

    if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        return sendConfig(setup.wLength);

    set_limits(setup.wLength);

    if (type == USB_DEVICE_DESCRIPTOR_TYPE)
        return send((uint8_t *)&device_descriptor, sizeof(DeviceDescriptor));

    else if (type == USB_STRING_DESCRIPTOR_TYPE)
    {
        // check if we exceed our bounds.
        if(setup.wValueL > STRING_DESCRIPTOR_COUNT - 1)
            return DEVICE_NOT_SUPPORTED;

        // send the string descriptor the host asked for.
        return send((uint8_t *)&string_descriptors[setup.wValueL], sizeof(string_descriptors[setup.wValueL]));
    }

    return DEVICE_OK;
}

CodalUSB::CodalUSB()
{
    usbInstance = this;
}

CodalUSB* CodalUSB::getInstance()
{
    if(usbInstance == NULL)
        usbInstance = new CodalUSB;

    return usbInstance;
}

int CodalUSB::configureEndpoints()
{
    uint8_t endpointCount = 1;

    InterfaceList* tmp = NULL;
    struct list_head *iter, *q = NULL;
    USBEndpoint* ep = NULL;

    uint8_t i = 0;
    uint8_t iEpCount = 0;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        iEpCount = tmp->interface->getEndpointCount();
        ep = tmp->interface->getEndpoints();

        for(i = 0; i < iEpCount; i++)
            if(init_endpoint(endpointCount + i, ep[i].type, ep[i].size) < 0 || reset_endpoint(endpointCount + i) < 0)
                return DEVICE_NOT_SUPPORTED;

        endpointCount += iEpCount;
    }

    return DEVICE_OK;
}

int CodalUSB::add(CodalUSBInterface& interface)
{
    uint8_t epsConsumed = interface.getEndpointCount();

    // - 2 for EP 0 (CONTROL)
    if(endpointsUsed + epsConsumed > DEVICE_USB_ENDPOINTS - 2)
        return DEVICE_NO_RESOURCES;

    InterfaceList* tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list)
        tmp = list_entry(iter, InterfaceList, list);

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

int CodalUSB::classRequest(USBSetup& setup)
{
    InterfaceList* tmp = NULL;
    struct list_head *iter, *q = NULL;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);
        tmp->interface->classRequest(setup);

        //if( == DEVICE_OK)
        //    return DEVICE_OK;
    }

    return DEVICE_OK;
}

int CodalUSB::endpointRequest(uint8_t endpoint)
{
    InterfaceList* tmp = NULL;
    struct list_head *iter, *q = NULL;

    uint8_t endpointCount = 1;
    uint8_t iEpCount = 0;
    uint8_t i = 0;

    list_for_each_safe(iter, q, &usb_list)
    {
        tmp = list_entry(iter, InterfaceList, list);

        iEpCount = tmp->interface->getEndpointCount();

        for(i = 0; i < iEpCount; i++)
        {
            if(endpointCount == endpoint)
            {
                tmp->interface->endpointRequest(endpoint, i);
                return DEVICE_OK;
            }

            endpointCount++;
        }
    }

    return DEVICE_NOT_SUPPORTED;
}

int CodalUSB::start()
{
    if(DEVICE_USB_ENDPOINTS == 0)
        return DEVICE_NOT_SUPPORTED;

    configure();

    UDIEN = (1<<EORSTE);

    return DEVICE_OK;
}
#endif
