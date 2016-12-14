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

#undef ENABLE
#undef DISABLE

static void gclk_sync(void) {
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
}

static void dfll_sync(void) {
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0)
        ;
}

#define CPU_FREQUENCY 48000000
static void mysystem_init(void) {
    NVMCTRL->CTRLB.bit.RWS = 1;

    SYSCTRL->XOSC32K.reg =
        SYSCTRL_XOSC32K_STARTUP(6) | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K;
    SYSCTRL->XOSC32K.bit.ENABLE = 1;
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0)
        ;

    GCLK->GENDIV.reg = GCLK_GENDIV_ID(1);
    gclk_sync();

    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN;
    gclk_sync();

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
    gclk_sync();

    SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
    dfll_sync();

    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(31) | SYSCTRL_DFLLMUL_FSTEP(511) |
                           SYSCTRL_DFLLMUL_MUL((CPU_FREQUENCY / (32 * 1024)));
    dfll_sync();

    SYSCTRL->DFLLCTRL.reg |=
        SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK | SYSCTRL_DFLLCTRL_QLDIS;
    dfll_sync();

    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;

    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 ||
           (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0)
        ;
    dfll_sync();

    GCLK->GENDIV.reg = GCLK_GENDIV_ID(0);
    gclk_sync();

    GCLK->GENCTRL.reg =
        GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
    gclk_sync();
}


void usb_configure(uint8_t numEndpoints)
{
    mysystem_init();

    usb_assert(usb_num_endpoints == 0);
    usb_assert(numEndpoints > 0);

    usb_num_endpoints = numEndpoints;
    usb_endpoints = new UsbDeviceDescriptor[usb_num_endpoints];
    memset(usb_endpoints, 0, usb_num_endpoints * sizeof(UsbDeviceDescriptor));

    uint32_t pad_transn, pad_transp, pad_trim;

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

    memset(usb_endpoints, 0, usb_num_endpoints * sizeof(UsbDeviceDescriptor));

    USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;

    system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_USB);

    USB->HOST.CTRLA.bit.ENABLE = true;
}

extern "C" void USB_Handler(void)
{
    if (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_EORST)
    {
        DMESG("USB EORST");
        /* Clear the flag */
        USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
        /* Set Device address as 0 */
        USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | 0;

        CodalUSB::usbInstance->initCtrlEndpoints();
        return;
    }

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
    flags = 0;

    UsbDeviceEndpoint *dep = &USB->DEVICE.DeviceEndpoint[ep];

    // Atmel type 0 is disabled, so types are shifted by 1
    dep->EPCFG.reg =
        USB_DEVICE_EPCFG_EPTYPE1(type + 1) | (dep->EPCFG.reg & USB_DEVICE_EPCFG_EPTYPE0_Msk);
    /* Set maximum packet size as 64 bytes */
    usb_endpoints[ep].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
    dep->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;

    // dep->EPINTENSET.reg =
    //    USB_DEVICE_EPINTENSET_TRCPT1 | USB_DEVICE_EPINTENSET_TRFAIL1 |
    //    USB_DEVICE_EPINTENSET_STALL1;
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

    // dep->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0 | USB_DEVICE_EPINTENSET_TRFAIL0 |
    //                      USB_DEVICE_EPINTENSET_STALL0 | USB_DEVICE_EPINTENSET_RXSTP;
    dep->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0 | USB_DEVICE_EPINTENSET_RXSTP;

    startRead();
}

void UsbEndpointOut::startRead()
{
    UsbDeviceDescriptor *epdesc = (UsbDeviceDescriptor *)usb_endpoints + ep;

    epdesc->DeviceDescBank[0].ADDR.reg = (uint32_t)buf;
    epdesc->DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
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

        DMESG("USBRead(%d) => %d bytes", ep, packetSize);

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

int UsbEndpointIn::write(const void *src, int len)
{
    uint32_t data_address;

    UsbDeviceDescriptor *epdesc = (UsbDeviceDescriptor *)usb_endpoints + ep;

    /* Check for requirement for multi-packet or auto zlp */
    if (len >= (1 << (epdesc->DeviceDescBank[1].PCKSIZE.bit.SIZE + 3)))
    {
        /* Update the EP data address */
        data_address = (uint32_t)src;
        // data must be in RAM!
        usb_assert(data_address >= HMCRAMC0_ADDR);

        epdesc->DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP = !(flags & USB_EP_FLAG_NO_AUTO_ZLP);
    }
    else
    {
        /* Copy to local buffer */
        memcpy(buf, src, len);
        data_address = (uint32_t)buf;
    }

    epdesc->DeviceDescBank[1].ADDR.reg = data_address;
    epdesc->DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = len;
    epdesc->DeviceDescBank[1].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
    /* Clear the transfer complete flag  */
    USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1;
    /* Set the bank as ready */
    USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK1RDY;

    /* Wait for transfer to complete */
    while (!(USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_TRCPT1))
    {
    }

    return len;
}
