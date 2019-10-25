#include "MicroBit.h"
#include "Tests.h"

MicroBit uBit;

#ifdef FULL_TEST
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

#endif

#ifdef DISPLAY_ON
void
raw_hello()
{
    while(1)
        uBit.display.scroll("GOOD MORNING!   ", 90);
}
#endif

void
raw_blinky_test()
{
    uBit.io.row1.setDigitalValue(0);
    uBit.io.row2.setDigitalValue(0);
    uBit.io.row3.setDigitalValue(0);
    uBit.io.row4.setDigitalValue(0);
    uBit.io.row5.setDigitalValue(0);

    uBit.io.col1.setDigitalValue(0);
    uBit.io.col2.setDigitalValue(0);
    uBit.io.col3.setDigitalValue(0);
    uBit.io.col4.setDigitalValue(0);
    uBit.io.col5.setDigitalValue(0);

    while(1)
    {
        uBit.io.row1.setDigitalValue(1);
        uBit.io.row2.setDigitalValue(1);
        uBit.io.row3.setDigitalValue(1);
        uBit.io.row4.setDigitalValue(1);
        uBit.io.row5.setDigitalValue(1);
        uBit.serial.printf("LED: ON...\n");
        uBit.sleep(500);

        uBit.io.row1.setDigitalValue(0);
        uBit.io.row2.setDigitalValue(0);
        uBit.io.row3.setDigitalValue(0);
        uBit.io.row4.setDigitalValue(0);
        uBit.io.row5.setDigitalValue(0);
        uBit.serial.printf("LED: OFF...\n");
        uBit.sleep(500);
    }
}

int 
main()
{
    uBit.sleep(1000);

    uBit.init();

#ifdef HIGH_DRIVE_TEST
    uBit.display.print(6);
    while(1){

        for (NRF52Pin *p : uBit.ledRowPins)
            p->setHighDrive(true);

        for (NRF52Pin *p : uBit.ledColPins)
            p->setHighDrive(true);
 
        uBit.sleep(1000);

        for (NRF52Pin *p : uBit.ledRowPins)
            p->setHighDrive(false);

        for (NRF52Pin *p : uBit.ledColPins)
            p->setHighDrive(false);

        uBit.sleep(1000);
    }
#endif
 
    for (int i=9; i>0; i--)
    {
        uBit.display.print(i);
        uBit.sleep(1000);
    }

    uBit.serial.printf("---- START ----\n");
    int i=0;

    while(1)
    {
        i++;
        uBit.serial.printf("FXOS:  ACC [X:%d][Y:%d][Z:%d]\n", uBit.fxosAccelerometer.getX(), uBit.fxosAccelerometer.getY(), uBit.fxosAccelerometer.getZ());
        uBit.serial.printf("LSM303 ACC [X:%d][Y:%d][Z:%d]\n", uBit.lsmAccelerometer.getX(), uBit.lsmAccelerometer.getY(), uBit.lsmAccelerometer.getZ());
        uBit.serial.printf("FXOS:  MAG [X:%d][Y:%d][Z:%d]\n", uBit.fxosCompass.getX(), uBit.fxosCompass.getY(), uBit.fxosCompass.getZ());
        uBit.serial.printf("LSM303 MAG [X:%d][Y:%d][Z:%d]\n", uBit.lsmCompass.getX(), uBit.lsmCompass.getY(), uBit.lsmCompass.getZ());

        uBit.sleep(1000);
    }


    //
    // SMOKETESTS: uncomment the ONE that you want.
    //

    //mems_mic_test();

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
    //demo();

    //display_wink();
    //test_read(0x0B, true);
    //test_read(0x0B, false);
    //test_read(0x00, true);

    while(1)
    {
        uBit.sleep(10000);
    }

}


