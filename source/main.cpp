#include "MicroBit.h"
#include "Tests.h"
#include "rtx_lib.h"

// Create a global instance of the MicroBit model called uBit.
// Allows this singleton to be accessed consistently by tests/programs.

#ifdef MICROBIT_UBIT_AS_STATIC_OBJECT
// A statically allocated model can be simply created using the code below.
// This is the simplest, and ideal for C/C++ programs.
MicroBit uBit;

#else
// Alternatively, we can dynamically allocated the model on te heap.
// This is better for testing builds for environments that do this (such as MakeCode) 
MicroBit &uBit = *(new MicroBit());
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

int 
main()
{
    uBit.sleep(100);
    uBit.init();

    DMESG("---- START ----\n");

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

    //
    // Default behaviour, in case nothing uncommented above.
    //    
    while(1)
        display_wink();
}