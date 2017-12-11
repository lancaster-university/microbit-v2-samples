#include "MicroBit.h"
#include "Tests.h"

MicroBit uBit;

void 
demo()
{
    int piezo =  isPiezoMic2();
    DMESG("MIC: %s", piezo ? "PIEZO" : "MEMS");

    display_wink();

    spirit_level2();

    display_arrows();
    button_blinky_test2();

    uBit.display.clear();

    if (piezo)
        piezo_clap_test(1);
    else
        mems_clap_test(1);

    display_tick();
    speaker_test2(3);

    record_factory_pass();

    uBit.sleep(2000);

    out_of_box_experience();
}


void 
factory_test()
{
    int piezo =  isPiezoMic2();
    DMESG("MIC: %s", piezo ? "PIEZO" : "MEMS");

    if (!hasPassedFactoryTests() || uBit.buttonB.isPressed())
    {
        display_wink();

        spirit_level2();

        display_arrows();
        button_blinky_test2();

        display_radio();
        radio_rx_test2();
        uBit.display.clear();

        if (piezo)
            piezo_clap_test(1);
        else
            mems_clap_test(1);

        display_tick();
        speaker_test2(3);

        record_factory_pass();

        uBit.sleep(2000);
    }

    out_of_box_experience();
}

void
test_read(uint8_t base, bool repeated_start)
{
    unsigned char buf[10];

    for (int i=0; i<10; i++)
        buf[i] = i;

    uBit.serial.printf("\n\n *** test_read: [repeated_start:%s] ***\n", repeated_start ? "true" : "false");
    for (int addr = base; addr < base+5; addr++)
    {
        uBit.serial.printf("\n\n -- 0x%.2X : 0x%.2X -- \n", addr, addr+5);
        uBit.i2c.readRegister(0x3C, addr, buf, 5, repeated_start);

        for (int i=0; i<5; i++)
            uBit.serial.printf("0x%.2X: [0x%.2X]\n", addr+i, buf[i]);
    }
}


int main()
{
    uBit.init();

    DMESG("INIT\n");
    uBit.sleep(100);

    //
    // SMOKETESTS: uncomment the ONE that you want.
    //

    //button_blinky_test();
    //display_test2();
    //spirit_level();
    //speaker_test();
    //edge_connector_test();
    //analog_test();
    //piezo_mic_test();
    //piezo_clap_test();
    //mems_mic_test();
    //mems_clap_test();
    //fade_test();
    //showSerialNumber();
    //square_wave_test();
    //button_test3();


    //factory_radio_transmitter();
    //factory_test(); 
    demo();

    //display_wink();
    //test_read(0x0B, true);
    //test_read(0x0B, false);
    //test_read(0x00, true);

    while(1)
        uBit.sleep(1000);

}


