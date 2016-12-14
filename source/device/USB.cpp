#include "CodalUSB.h"
#include "USB.h"

static UsbDeviceDescriptor *usb_endpoints;
static uint8_t usb_num_endpoints;

#define NVM_USB_PAD_TRANSN_POS 45
#define NVM_USB_PAD_TRANSN_SIZE 5
#define NVM_USB_PAD_TRANSP_POS 50
#define NVM_USB_PAD_TRANSP_SIZE 5
#define NVM_USB_PAD_TRIM_POS 55
#define NVM_USB_PAD_TRIM_SIZE 3

void usb_configure(uint8_t numEndpoints)
{
    uint32_t pad_transn, pad_transp, pad_trim;

    usb_assert(usb_num_endpoints == 0);
    usb_assert(numEndpoints > 0);

    usb_num_endpoints = numEndpoints;
    usb_endpoints = new UsbDeviceDescriptor[usb_num_endpoints];
    memset(usb_endpoints, 0, usb_num_endpoints * sizeof(UsbDeviceDescriptor));

    /* Enable USB clock */
    PM->APBBMASK.reg |= PM_APBBMASK_USB;

    /* Set up the USB DP/DN pins */
    PORT->Group[0].PINCFG[PIN_PA24G_USB_DM].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[PIN_PA24G_USB_DM / 2].reg &= ~(0xF << (4 * (PIN_PA24G_USB_DM & 0x01u)));
    PORT->Group[0].PMUX[PIN_PA24G_USB_DM / 2].reg |= MUX_PA24G_USB_DM
                                                     << (4 * (PIN_PA24G_USB_DM & 0x01u));
    PORT->Group[0].PINCFG[PIN_PA25G_USB_DP].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[PIN_PA25G_USB_DP / 2].reg &= ~(0xF << (4 * (PIN_PA25G_USB_DP & 0x01u)));
    PORT->Group[0].PMUX[PIN_PA25G_USB_DP / 2].reg |= MUX_PA25G_USB_DP
                                                     << (4 * (PIN_PA25G_USB_DP & 0x01u));

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(6) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;

    /* Reset */
    USB->HOST.CTRLA.bit.SWRST = 1;
    while (USB->HOST.SYNCBUSY.bit.SWRST)
    {
        /* Sync wait */
    }

    /* Load Pad Calibration */
    pad_transn = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_USB_PAD_TRANSN_POS / 32)) >>
                  (NVM_USB_PAD_TRANSN_POS % 32)) &
                 ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);

    if (pad_transn == 0x1F)
    {
        pad_transn = 5;
    }

    USB->HOST.PADCAL.bit.TRANSN = pad_transn;

    pad_transp = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_USB_PAD_TRANSP_POS / 32)) >>
                  (NVM_USB_PAD_TRANSP_POS % 32)) &
                 ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);

    if (pad_transp == 0x1F)
    {
        pad_transp = 29;
    }

    USB->HOST.PADCAL.bit.TRANSP = pad_transp;
    pad_trim = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_USB_PAD_TRIM_POS / 32)) >>
                (NVM_USB_PAD_TRIM_POS % 32)) &
               ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);

    if (pad_trim == 0x7)
    {
        pad_trim = 3;
    }

    USB->HOST.PADCAL.bit.TRIM = pad_trim;

    /* Set the configuration */
    /* Set mode to Device mode */
    USB->HOST.CTRLA.bit.MODE = 0;
    /* Enable Run in Standby */
    USB->HOST.CTRLA.bit.RUNSTDBY = true;
    /* Set the descriptor address */
    USB->HOST.DESCADD.reg = (uint32_t)(&usb_endpoints[0]);
    /* Set speed configuration to Full speed */
    USB->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_FS_Val;
    /* Attach to the USB host */
    USB->DEVICE.CTRLB.reg &= ~USB_DEVICE_CTRLB_DETACH;

    // USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;

    system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_USB);
}

extern "C" void USB_Handler(void)
{
    CodalUSB::usbInstance->interruptHandler();
}

bool usb_recieved_setup()
{
    return USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_RXSTP;
}

void usb_clear_setup()
{
    USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;
}

void usb_set_address(uint16_t wValue)
{
    USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | wValue;
}

int UsbEndpointIn::stall()
{
    USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ1;
    return 0;
}

int UsbEndpointOut::stall()
{
    USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ0;
    return 0;
}

UsbEndpointIn::UsbEndpointIn(uint8_t idx, uint8_t type, uint8_t size)
{
    usb_assert(size == 64);
    usb_assert(type <= USB_EP_TYPE_INTERRUPT);
    ep = idx;

    UsbDeviceEndpoint *dep = &USB->DEVICE.DeviceEndpoint[ep];

    // Atmel type 0 is disabled, so types are shifted by 1
    dep->EPCFG.reg =
        USB_DEVICE_EPCFG_EPTYPE1(type + 1) | (dep->EPCFG.reg & USB_DEVICE_EPCFG_EPTYPE0_Msk);
    /* Set maximum packet size as 64 bytes */
    usb_endpoints[ep].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
    dep->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;

    dep->EPINTENSET.reg =
        USB_DEVICE_EPINTENSET_TRCPT1 | USB_DEVICE_EPINTENSET_TRFAIL1 | USB_DEVICE_EPINTENSET_STALL1;
}

UsbEndpointOut::UsbEndpointOut(uint8_t idx, uint8_t type, uint8_t size)
{
    usb_assert(size == 64);
    usb_assert(type <= USB_EP_TYPE_INTERRUPT);
    ep = idx;

    UsbDeviceEndpoint *dep = &USB->DEVICE.DeviceEndpoint[ep];

    dep->EPCFG.reg =
        USB_DEVICE_EPCFG_EPTYPE0(type + 1) | (dep->EPCFG.reg & USB_DEVICE_EPCFG_EPTYPE1_Msk);
    /* Set maximum packet size as 64 bytes */
    usb_endpoints[ep].DeviceDescBank[0].PCKSIZE.bit.SIZE = 3;
    usb_endpoints[ep].DeviceDescBank[0].ADDR.reg = (uint32_t)buf;
    dep->EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK0RDY;

    dep->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0 | USB_DEVICE_EPINTENSET_TRFAIL0 |
                          USB_DEVICE_EPINTENSET_STALL0 | USB_DEVICE_EPINTENSET_RXSTP;

    startRead();
}

void UsbEndpointOut::startRead()
{
    UsbDeviceDescriptor *epdesc = (UsbDeviceDescriptor *)usb_endpoints + ep;

    /* Set the buffer address for ep data */
    epdesc->DeviceDescBank[0].ADDR.reg = (uint32_t)buf;
    /* Set the byte count as zero */
    epdesc->DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
    /* Set the byte count as zero */
    epdesc->DeviceDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
    /* Start the reception by clearing the bank 0 ready bit */
    USB->DEVICE.DeviceEndpoint[ep].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSSET_BK0RDY;
}

int UsbEndpointOut::read(void *dst, int maxlen)
{
    int packetSize = 0;

    /* Check for Transfer Complete 0 flag */
    if (USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_TRCPT0)
    {
        packetSize = usb_endpoints[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;

        // Note that we shall discard any excessive data
        if (packetSize > maxlen)
            packetSize = maxlen;

        memcpy(dst, buf, packetSize);

        /* Clear the Transfer Complete 0 flag */
        USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0;

        startRead();
    }

    return packetSize;
}

#if 0
extern const DeviceDescriptor device_descriptor = {18, 1,  0x0002, 0,      0,
                                                   0,  64, 0x0B6A, 0x5346, 0x0001,
                                                   1, // product ID
                                                   2, // manufacturer ID
                                                   3, // serial number
                                                   1};

extern const StringDescriptor string_descriptors[STRING_DESCRIPTOR_COUNT] = {
    /*! index 0 - language */
    {
        4,           // length
        3,           // descriptor type - string
        {0x09, 0x04} // languageID - United States
    },
    /*! index 1 - product ID */
    {18, // length
     3,  // descriptor type - string
     {'A', 0, 't', 0, 'm', 0, 'e', 0, 'g', 0, 'a', 0, '3', 0, '2', 0}},
    /*! index 2 - manufacturer ID */
    {12, // length
     3,  // descriptor type - string
     {'J', 0, 'A', 0, 'M', 0, 'E', 0, 'S', 0}},
    /*! index 3 - serial number */
    {26, // length
     3,  // descriptor type - string
     {'1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0,
      '7', 0, '8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0}}};
#endif
