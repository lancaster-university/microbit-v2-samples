#ifndef DEVICE_USB_H
#define DEVICE_USB_H

#if CONFIG_ENABLED(DEVICE_USB)

#include <avr/io.h>

#define DEVICE_USB_ENDPOINTS    6

#define EP_SINGLE_64            0x32
#define EP_DOUBLE_64            0x36
#define EP_SINGLE_16            0x12

#define EP_TYPE_CONTROL                 (0x00)
#define EP_TYPE_BULK_IN                 ((1<<EPTYPE1) | (1<<EPDIR))
#define EP_TYPE_BULK_OUT                (1<<EPTYPE1)
#define EP_TYPE_INTERRUPT_IN            ((1<<EPTYPE1) | (1<<EPTYPE0) | (1<<EPDIR))
#define EP_TYPE_INTERRUPT_OUT           ((1<<EPTYPE1) | (1<<EPTYPE0))
#define EP_TYPE_ISOCHRONOUS_IN          ((1<<EPTYPE0) | (1<<EPDIR))
#define EP_TYPE_ISOCHRONOUS_OUT         (1<<EPTYPE0)

static inline void usb_reset()
{
    
}



static inline void configure_usb()
{
    int32_t pad_transn, pad_transp, pad_trim;

    PM->APBBMASK.reg |= PM_APBBMASK_USB;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
    GCLK_CLKCTRL_GEN(USB_GCLK_GEN) |
    GCLK_CLKCTRL_ID(USB_GCLK_ID);

    /* Reset */
    USB->DEVICE.CTRLA.reg = USB_CTRLA_SWRST;
    while (USB->DEVICE.SYNCBUSY.bit.SWRST);

    USB->DEVICE.CTRLA.reg = USB_CTRLA_ENABLE | USB_CTRLA_MODE_DEVICE;
    while (USB->DEVICE.SYNCBUSY.bit.ENABLE);

    /* Load Pad Calibration */
    pad_transn = ( *((uint32_t *)(NVMCTRL_OTP4)
    + (NVM_USB_PAD_TRANSN_POS / 32))
    >> (NVM_USB_PAD_TRANSN_POS % 32))
    & ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);

    if (pad_transn == 0x1F) {
    pad_transn = 5;
    }

    pad_transp =( *((uint32_t *)(NVMCTRL_OTP4)
    + (NVM_USB_PAD_TRANSP_POS / 32))
    >> (NVM_USB_PAD_TRANSP_POS % 32))
    & ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);

    if (pad_transp == 0x1F) {
    pad_transp = 29;
    }

    pad_trim =( *((uint32_t *)(NVMCTRL_OTP4)
    + (NVM_USB_PAD_TRIM_POS / 32))
    >> (NVM_USB_PAD_TRIM_POS % 32))
    & ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);

    if (pad_trim == 0x7) {
    pad_trim = 3;
    }

    USB->DEVICE.PADCAL.reg = USB_PADCAL_TRANSN(pad_transn) | USB_PADCAL_TRANSP(pad_transp) | USB_PADCAL_TRIM(pad_trim);

    memset(usb_endpoints, 0, usb_num_endpoints*sizeof(UsbDeviceDescriptor));
    USB->DEVICE.DESCADD.reg = (uint32_t)(&usb_endpoints[0]);
    USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;

    usb_reset();
}

static inline uint8_t read8()
{
    return UEDATX;
}

static inline void send8(uint8_t data)
{
    UEDATX = data;
}

static inline uint8_t recieved_setup()
{
    return UEINTX & (1 << RXSTPI);
}

static inline void wait_tx_int(void)
{
    while (!(UEINTX & (1<<TXINI)));
}

static inline void clear_tx_int(void)
{
    UEINTX = ~(1<<TXINI);
}

static inline void wait_rx_int(void)
{
    while (!(UEINTX & (1 << RXOUTI)));
}

static inline uint8_t wait_rx_tx_int()
{
    while (!(UEINTX & ((1<<TXINI)|(1<<RXOUTI))));
    return (UEINTX & (1<<RXOUTI)) == 0;
}

static inline void clear_rx_int(void)
{
    UEINTX = ~(1 << RXOUTI);
}

static inline void clear_setup()
{
    UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
}

static inline void stall()
{
    EPSTATUS = 1 << STALLRQ;
}

static inline void set_endpoint(uint8_t ep)
{
    UENUM = ep;
}

static inline int get_endpoint()
{
    for(uint8_t i = 0; i < 8; i++)
        if(UEINT & _BV(i))
            return i;

    return -1;
}

static inline uint8_t get_frame_number()
{
    return UDFNUML;
}

int init_endpoint(uint8_t index, uint8_t type, uint8_t size);

int reset_endpoint(uint8_t index);

#endif
#endif
