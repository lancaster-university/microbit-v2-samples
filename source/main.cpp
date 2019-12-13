#include "MicroBit.h"
#include "Tests.h"
#include "rtx_lib.h"

MicroBit *uBitPointer = new MicroBit();
MicroBit &uBit = *uBitPointer;

//MicroBit uBit;

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

#endif
void 
factory_test()
{
    DMESG("Checking tests...");
    if (!hasPassedFactoryTests() || uBit.buttonB.isPressed())
    {
      DMESG("Running tests...");
      display_wink();

      spirit_level2();

      display_arrows();
      button_blinky_test2();

      display_radio();
      radio_rx_test2();
      uBit.display.clear();
      mems_clap_test(1);
      display_tick();
      speaker_test2(3);
      uBit.sleep(2000);
      record_factory_pass();
    }
    DMESG("Running OOB");
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

void doSomething(int i);
void doSomething2(int i);
void doSomethingElse(int i);

void doSomethingElse(int i)
{
    DMESG("SP:ELSE:%d: %p", i, get_current_sp());
}


void doSomething2(int i)
{
    DMESG("SP:B:%d: %p", i, get_current_sp());
    doSomethingElse(i);
}

void doSomething(int i)
{
    while (i<10){
        DMESG("SP:A:%d: %p", i, get_current_sp());
        doSomething2(i);
        i++;
    }
}

extern void device_heap_print();

#ifdef POOP
int 
main()
{
    uBit.sleep(100);
    
    uBit.init();

    osThreadId_t id = osThreadGetId();
    osRtxThread_t *thread = osRtxThreadId(id);

    DMESG("STACK_MEM: %p, STACK_SIZE: %d", (uint32_t)thread->stack_mem, (uint32_t)thread->stack_size);
    DMESG("SP0: %p", get_current_sp());
    doSomething(0);

    device_heap_print();


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

#ifdef STARTUP_COUNTDOWN
    for (int i=9; i>0; i--)
    {
        uBit.display.print(i);
        uBit.sleep(1000);
    }
#endif

    uBit.serial.printf("---- START ----\n");

    //
    // SMOKETESTS: uncomment the ONE that you want.
    //

    //mems_mic_test();
    //while(1) {
        //concurrent_display_test();
        //button_test3();
	    //display_test1();
        //button_blinky_test2();
	    //speaker_test2(2);
	    //mems_clap_test(3);
	    //spirit_level2();
	    //edge_connector_test();
	    //analog_test();
	    //piezo_mic_test();
	    //piezo_clap_test();
	    //mems_mic_test();
	    //fade_test();
	    //showSerialNumber();
	    //square_wave_test();
    //}

    //factory_radio_transmitter();
    //factory_test(); 
    //demo();

    //display_wink();
    //test_read(0x0B, true);
    //test_read(0x0B, false);
    //test_read(0x00, true);

const char * const happy_emoji ="\
    000,255,000,255,000\n\
    000,000,000,000,000\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

const char * const wink_emoji ="\
    000,255,000,000,000\n\
    000,000,000,000,000\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

    MicroBitImage smile(happy_emoji);
    MicroBitImage wink(wink_emoji);

    while(1)
    {
        //display_wink();

        DMESGN("ON...");
        //uBit.display.print.setPixelValue(0,0,255);
        uBit.display.print(smile);

        uBit.sleep(500);

        DMESG("OFF");
        //uBit.display.image.setPixelValue(0,0,0);
        uBit.display.print(wink);
        
        uBit.sleep(500);
    }
}
#endif

void
onButtonAPressed(MicroBitEvent)
{
    const char * const a_emoji ="\
        255,000,000,000,255\n\
        000,000,000,000,000\n\
        255,255,255,255,255\n\
        000,000,000,255,255\n\
        000,000,000,255,255\n";

    MicroBitImage img_a(a_emoji);
    uBit.display.print(img_a);
}

void
onButtonBPressed(MicroBitEvent)
{
    const char * const b_emoji ="\
        000,255,000,000,255\n\
        000,000,255,255,000\n\
        000,000,255,255,000\n\
        000,000,255,255,000\n\
        000,255,000,000,255\n";
    MicroBitImage img_b(b_emoji);
    uBit.display.print(img_b);    
}

void
onButtonABPressed(MicroBitEvent)
{
    const char * const c_emoji ="\
        000,000,000,255,255\n\
        000,000,000,255,255\n\
        255,255,255,255,255\n\
        255,255,255,255,255\n\
        000,255,000,255,000\n";
    MicroBitImage img_c(c_emoji);
    uBit.display.print(img_c);    

}

void
onShakePressed(MicroBitEvent)
{
    const char * const d_emoji ="\
        255,000,000,000,255\n\
        000,255,000,255,000\n\
        000,000,000,000,000\n\
        255,255,255,255,255\n\
        255,000,255,000,255\n";
    MicroBitImage img_d(d_emoji);
    uBit.display.print(img_d);    

}

void
do_something_forever()
{
    uBit.sleep(10);
}

int 
main()
{
    uBit.sleep(100);    
    uBit.init();

    const char * const heart_emoji ="\
    000,255,000,255,000\n\
    255,255,255,255,255\n\
    255,255,255,255,255\n\
    000,255,255,255,000\n\
    000,000,255,000,000\n";

    MicroBitImage img_heart(heart_emoji);
    uBit.display.print(img_heart);

    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonAPressed);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonBPressed);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_CLICK, onButtonABPressed);
    uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_ACCELEROMETER_EVT_SHAKE, onShakePressed);

    create_fiber(do_something_forever);
    release_fiber();
}


