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
off_test()
{
    uBit.power.off();
}

static void power_management_enter_standby(MicroBitEvent)
{
    DMESG("Entering System Off in 1 second...");
    uBit.sleep(1000);

    uBit.power.off();
}

static void power_management_deep_sleep_until_button_b(MicroBitEvent)
{
    DMESG("Entering Deep Sleep, wake on button B.");
    uBit.io.buttonB.setActiveLo();
    uBit.io.buttonB.wakeOnActive(1);
    uBit.power.deepSleep();
    DMESG("Leaving Deep Sleep...");
}

static void power_management_deep_sleep_until_P0_high(MicroBitEvent)
{
    DMESG("Entering Deep Sleep, on P0 LO->HI");
    uBit.io.P0.setPull(PullMode::Down);
    uBit.io.P0.setActiveHi();
    uBit.io.P0.wakeOnActive(1);
    uBit.power.deepSleep();
}

void
interactive_off_test()
{
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, power_management_enter_standby);
    uBit.display.print("*");

    while(1)
        uBit.sleep(10000);
}

void
deep_sleep_test1()
{
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, power_management_deep_sleep_until_button_b);
    uBit.display.print("*");

    while(1)
        uBit.sleep(10000);
}

void
deep_sleep_test2()
{
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, power_management_deep_sleep_until_P0_high);
    uBit.display.print("*");

    while(1)
        uBit.sleep(10000);
}

void
interactive_deep_sleep_test()
{
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, power_management_enter_standby);
    uBit.display.print("*");

    while(1)
        uBit.sleep(10000);
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