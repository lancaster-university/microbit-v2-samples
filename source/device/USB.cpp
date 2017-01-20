#include "CodalUSB.h"
#include "USB.h"

#if CONFIG_ENABLED(DEVICE_USB)

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

static void gclk_sync(void)
{
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
}

static void dfll_sync(void)
{
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0)
        ;
}

#define CPU_FREQUENCY 48000000
#define NVM_SW_CALIB_DFLL48M_COARSE_VAL 58
#define NVM_SW_CALIB_DFLL48M_FINE_VAL 64

static void mysystem_init(void)
{
    NVMCTRL->CTRLB.bit.RWS = 1;

    // Turn on DFLL with USB correction and sync to internal 8 mhz oscillator
    SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
    dfll_sync();

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    SYSCTRL_DFLLCTRL_Type dfllctrl_conf = {0};
    SYSCTRL_DFLLVAL_Type dfllval_conf = {0};
    uint32_t coarse = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_COARSE_VAL / 32)) >>
                       (NVM_SW_CALIB_DFLL48M_COARSE_VAL % 32)) &
                      ((1 << 6) - 1);
    if (coarse == 0x3f)
    {
        coarse = 0x1f;
    }
    uint32_t fine = (*((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_FINE_VAL / 32)) >>
                     (NVM_SW_CALIB_DFLL48M_FINE_VAL % 32)) &
                    ((1 << 10) - 1);
    if (fine == 0x3ff)
    {
        fine = 0x1ff;
    }
    dfllval_conf.bit.COARSE = coarse;
    dfllval_conf.bit.FINE = fine;
    dfllctrl_conf.bit.USBCRM = 1; // usb correction
    dfllctrl_conf.bit.BPLCKC = 0;
    dfllctrl_conf.bit.QLDIS = 0;
    dfllctrl_conf.bit.CCDIS = 1;
    dfllctrl_conf.bit.ENABLE = 1;

    SYSCTRL->DFLLMUL.reg = 48000;
    SYSCTRL->DFLLVAL.reg = dfllval_conf.reg;
    SYSCTRL->DFLLCTRL.reg = dfllctrl_conf.reg;

    GCLK->CLKCTRL.bit.ID = 0; // GCLK_ID - DFLL48M Reference
    gclk_sync();

    GCLK->CLKCTRL.bit.CLKEN = 1;
    GCLK->CLKCTRL.bit.WRTLOCK = 0;
    GCLK->CLKCTRL.bit.GEN = 0;

    // Configure DFLL48M as source for GCLK_GEN 0
    GCLK->GENDIV.bit.ID = 0; 
    gclk_sync();
    GCLK->GENDIV.reg = 0;

    GCLK->GENCTRL.bit.ID = 0; 
    gclk_sync();
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
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

    USB->DEVICE.INTENCLR.reg = USB_DEVICE_INTFLAG_MASK;
    USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;

    system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_USB);

    USB->HOST.CTRLA.bit.ENABLE = true;
}

extern "C" void USB_Handler(void)
{
    CodalUSB *cusb = CodalUSB::usbInstance;

#if 0
    DMESG("USB devint=%x ep0int=%x ep1int=%x", USB->DEVICE.INTFLAG.reg,
          USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg, USB->DEVICE.DeviceEndpoint[1].EPINTFLAG.reg);
#endif

    if (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_EORST)
    {
        DMESG("USB EORST");
        /* Clear the flag */
        USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
        /* Set Device address as 0 */
        USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | 0;

        cusb->initEndpoints();
        return;
    }

    if (USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_RXSTP)
    {
        // clear the flag
        USBSetup setup;
        int len = cusb->ctrlOut->read(&setup, sizeof(setup));
        usb_assert(len == sizeof(setup));
        USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;
        cusb->setupRequest(setup);
        return;
    }

    cusb->interruptHandler();
}

void usb_set_address(uint16_t wValue)
{
    USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | wValue;
}

int UsbEndpointIn::stall()
{
    DMESG("stall IN %d", ep);
    USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ1;
    wLength = 0;
    return DEVICE_OK;
}

int UsbEndpointOut::stall()
{
    DMESG("stall OUT %d", ep);
    USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ0;
    return DEVICE_OK;
}

UsbEndpointIn::UsbEndpointIn(uint8_t idx, uint8_t type, uint8_t size)
{
    usb_assert(size == 64);
    usb_assert(type <= USB_EP_TYPE_INTERRUPT);
    ep = idx;
    flags = 0;

    if (type == USB_EP_TYPE_INTERRUPT)
        flags = USB_EP_FLAG_NO_AUTO_ZLP;

    UsbDeviceEndpoint *dep = &USB->DEVICE.DeviceEndpoint[ep];

    // Atmel type 0 is disabled, so types are shifted by 1
    dep->EPCFG.reg =
        USB_DEVICE_EPCFG_EPTYPE1(type + 1) | (dep->EPCFG.reg & USB_DEVICE_EPCFG_EPTYPE0_Msk);
    /* Set maximum packet size as 64 bytes */
    usb_endpoints[ep].DeviceDescBank[1].PCKSIZE.bit.SIZE = 3;
    dep->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;

    dep->EPINTENCLR.reg = USB_DEVICE_EPINTFLAG_MASK;

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
    dep->EPINTENCLR.reg = USB_DEVICE_EPINTFLAG_MASK;
    if (idx == 0)
        dep->EPINTENSET.reg = USB_DEVICE_EPINTENSET_RXSTP;
    else
        dep->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0;

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

    usb_assert(this != NULL);

    uint32_t flag = ep == 0 ? USB_DEVICE_EPINTFLAG_RXSTP : USB_DEVICE_EPINTFLAG_TRCPT0;

    /* Check for Transfer Complete 0 flag */
    if (USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg & flag)
    {
        packetSize = usb_endpoints[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;

        // DMESG("USBRead(%d) => %d bytes", ep, packetSize);

        // Note that we shall discard any excessive data
        if (packetSize > maxlen)
            packetSize = maxlen;

        memcpy(dst, buf, packetSize);

        /* Clear the Transfer Complete 0 flag */
        USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = flag;

        startRead();
    }

    return packetSize;
}

int UsbEndpointIn::write(const void *src, int len)
{
    uint32_t data_address;

    // this happens when someone tries to write before USB is initialized
    usb_assert(this != NULL);

    UsbDeviceDescriptor *epdesc = (UsbDeviceDescriptor *)usb_endpoints + ep;

    if (wLength)
    {
        if (len > wLength)
            len = wLength;
        wLength = 0;
    }

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

    return DEVICE_OK;
}

#endif
