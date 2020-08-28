#include "MicroBit.h"
#include "Tests.h"

#include "MicroBitPowerManager.h"

    const char * const dc_emoji ="\
    000,000,000,000,000\n\
    000,000,000,000,000\n\
    255,255,255,255,255\n\
    000,000,000,000,000\n\
    000,000,000,000,000\n";

    const char * const usb_emoji ="\
    000,000,000,000,000\n\
    255,255,255,255,255\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

    const char * const battery_emoji ="\
    000,000,255,000,000\n\
    000,255,255,255,000\n\
    000,255,255,255,000\n\
    000,255,255,255,000\n\
    000,255,255,255,000\n";

    static MicroBitImage dc(dc_emoji);
    static MicroBitImage usb(usb_emoji);
    static MicroBitImage battery(battery_emoji);

void
version_test()
{
    MicroBitVersion v;

    v.board = 0;
    v.daplink = 0;
    v.i2c = 0;

    v = uBit.power.getVersion();
    DMESG("VERSION: [board: %d] [daplink: %d] [i2c-protocol %d]", v.board, v.daplink, v.i2c);  
}

void
standby_test()
{
    uBit.power.standby();
}

void
usb_connection_test()
{
    while(1)
    {
        uBit.display.print(" ");
        uBit.sleep(100);    
        DMESG("POWER_CONSUMPTION:  %d", uBit.power.getPowerConsumption());
        uBit.sleep(2000);    

        uBit.display.print("*");
        uBit.sleep(100);    
        DMESG("POWER_CONSUMPTION:  %d", uBit.power.getPowerConsumption());
        uBit.sleep(2000);    
    }
}

void
power_source_test()
{
    while(1)
    {
        MicroBitPowerSource p = uBit.power.getPowerSource();

        if (p == PWR_SOURCE_NONE)
        {
            DMESG("POWER_SOURCE:  NONE");
            uBit.display.print(dc);
            uBit.sleep(2000);
        }

        if (p == PWR_USB_ONLY)
        {
            DMESG("POWER_SOURCE:  USB");
            uBit.display.print(usb);
            uBit.sleep(2000);
        }

        if (p == PWR_BATT_ONLY)
        {
            DMESG("POWER_SOURCE:  BATTERY");
            uBit.display.print(battery);
            uBit.sleep(2000);

        }

        if (p == PWR_USB_AND_BATT)
        {
            DMESG("POWER_SOURCE:  USB+BATTERY");
            uBit.display.print(usb);
            uBit.sleep(500);
            uBit.display.print(battery);
            uBit.sleep(500);
            uBit.display.print(usb);
            uBit.sleep(500);
            uBit.display.print(battery);
            uBit.sleep(500);
        }
    }
}