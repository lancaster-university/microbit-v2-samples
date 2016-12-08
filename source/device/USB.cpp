#include "CodalUSB.h"
#include "USB.h"

extern const DeviceDescriptor device_descriptor =
{
    18,
    1,
    0x0002,
    0,
    0,
    0,
    64,
    0x0B6A,
    0x5346,
    0x0001,
    1,  // product ID
    2,  // manufacturer ID
    3,  // serial number
    1
};

extern const StringDescriptor string_descriptors[STRING_DESCRIPTOR_COUNT] =
{
    /*! index 0 - language */
    {
        4,              // length
        3,              // descriptor type - string
        {0x09,0x04}      // languageID - United States
    },
    /*! index 1 - product ID */
    {
        18,             // length
        3,              // descriptor type - string
        {'A',0,'t',0,'m',0,'e',0,'g',0,'a',0,'3',0,'2',0}
    },
    /*! index 2 - manufacturer ID */
    {
        12,             // length
        3,              // descriptor type - string
        {'J',0,'A',0,'M',0,'E',0,'S',0}
    },
    /*! index 3 - serial number */
    {   26,             // length
        3,              // descriptor type - string
        {'1',0,'2',0,'3',0,'4',0,'5',0,'6',0,'7',0,'8',0,'9',0,'A',0,'B',0,'C',0}
    }
};

int init_endpoint(uint8_t index, uint8_t type, uint8_t size)
{
    if(index > DEVICE_USB_ENDPOINTS)
        return DEVICE_NOT_SUPPORTED;

    UENUM = index;
    UECONX |= _BV(EPEN);
    UECFG0X = type;
    UECFG1X = size;

    if(type == EP_TYPE_BULK_OUT)
        UEIENX |= _BV(RXOUTE);

    if(type == EP_TYPE_BULK_IN)
        UEIENX |= _BV(TXINI);

    return DEVICE_OK;
}

int reset_endpoint(uint8_t index)
{
    if(index > DEVICE_USB_ENDPOINTS)
        return DEVICE_NOT_SUPPORTED;

    uint8_t bit = (1 << index);

    UERST |= bit;
    UERST &= ~bit;

    return DEVICE_OK;
}
