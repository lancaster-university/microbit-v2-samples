
#include "MicroBit.h"
#include "Tests.h"

////////////////////////////////////////////////////////////////
// TESTS

// A fiber that sleeps for a few seconds
void deepsleep_test1();

 // A fiber that sleeps until woken by button A or B
void deepsleep_test2();

// A timer event handler that sleeps between events
void deepsleep_test3();

// Wake to run button A or B handlers
void deepsleep_test4();

// Two timer event handlers with different periods that sleep between events
void deepsleep_test5();

////////////////////////////////////////////////////////////////
// TEST

void deepsleep_test( int test)
{
    switch ( test)
    {
        case 1: deepsleep_test1(); break;
        case 2: deepsleep_test2(); break;
        case 3: deepsleep_test3(); break;
        case 4: deepsleep_test4(); break;
        case 5: deepsleep_test5(); break;
    }

    release_fiber();
}

////////////////////////////////////////////////////////////////
// HELPERS

void deepsleep_test_zeroone();
void deepsleep_test_threefour();
void deepsleep_test_send_time( const char *suffix);

////////////////////////////////////////////////////////////////
// TEST 1

// A fiber that sleeps for a few seconds

void deepsleep_test1_fiber()
{
    while (true)
    {
        deepsleep_test_send_time( "deepsleep_test1_fiber\n");
        deepsleep_test_zeroone();

        uBit.power.deepSleep(3000);
    }
}

void deepsleep_test1()
{
    deepsleep_test_send_time( "deepsleep_test2\n");

    // Show that display is off during sleep
    uBit.display.image.setPixelValue( 2, 0, 128);

    create_fiber( deepsleep_test1_fiber);
}

////////////////////////////////////////////////////////////////
// TEST 2

// A fiber that sleeps until woken by button A or B

void deepsleep_test2_fiber()
{
    while (true)
    {
        deepsleep_test_send_time( "deepsleep_test2_fiber\n");

        uBit.power.deepSleep();

        if ( uBit.buttonA.isPressed())
            uBit.display.print('A');
        else if ( uBit.buttonB.isPressed())
            uBit.display.print('B');
        else
            uBit.display.print('C');
    }
}


void deepsleep_test2()
{
    deepsleep_test_send_time( "deepsleep_test2\n");

    // Show that display is off during sleep
    uBit.display.image.setPixelValue( 2, 0, 128);

    uBit.io.buttonA.wakeOnActive(1);
    uBit.io.buttonB.wakeOnActive(1);

    create_fiber( deepsleep_test2_fiber);
}

////////////////////////////////////////////////////////////////
// TEST 3

// A timer event handler that sleeps between events

void deepsleep_test3_onTimer(MicroBitEvent e)
{
    deepsleep_test_send_time( "deepsleep_test3_onTimer\n");
    deepsleep_test_zeroone();

    // Request deep sleep and exit the handler
    uBit.power.deepSleepAsync();
}


void deepsleep_test3()
{
    deepsleep_test_send_time( "deepsleep_test3\n");

    // Show that display is off during sleep
    uBit.display.image.setPixelValue( 2, 0, 128);

    uint16_t        timer_id      = 60000;
    uint16_t        timer_value   = 1;
    CODAL_TIMESTAMP timer_period  = 5000; //ms

    uBit.messageBus.listen( timer_id, timer_value, deepsleep_test3_onTimer);

    // CODAL_TIMER_EVENT_FLAGS_WAKEUP makes the timer event trigger power up
    system_timer_event_every( timer_period, timer_id, timer_value, CODAL_TIMER_EVENT_FLAGS_WAKEUP);
}

////////////////////////////////////////////////////////////////
// TEST 4

// Wake to run button A or B handlers

void deepsleep_test4_onButtonA(MicroBitEvent e)
{
    // Disable deep sleep power down until we have finished
    uBit.power.powerDownDisable();

    deepsleep_test_zeroone();
    uBit.sleep(500);
    deepsleep_test_zeroone();

    uBit.power.powerDownEnable();
    uBit.power.deepSleepAsync();
}


void deepsleep_test4_onButtonB(MicroBitEvent e)
{
    uBit.power.powerDownDisable();

    deepsleep_test_threefour();
    uBit.sleep(500);
    deepsleep_test_threefour();

    uBit.power.powerDownEnable();
    uBit.power.deepSleepAsync();
}


void deepsleep_test4()
{
    deepsleep_test_send_time( "deepsleep_test4\n");

    // Show that display is off during sleep
    uBit.display.image.setPixelValue( 2, 0, 128);

    uBit.messageBus.listen( MICROBIT_ID_BUTTON_A,  MICROBIT_BUTTON_EVT_DOWN, deepsleep_test4_onButtonA);
    uBit.messageBus.listen( MICROBIT_ID_BUTTON_B,  MICROBIT_BUTTON_EVT_DOWN, deepsleep_test4_onButtonB);

    uBit.io.buttonA.wakeOnActive(1);
    uBit.io.buttonB.wakeOnActive(1);

    uBit.power.deepSleepAsync();
}

////////////////////////////////////////////////////////////////
// TEST 5

// Two timer event handlers with different periods that sleep between events

void deepsleep_test5_onTimer1(MicroBitEvent e)
{
    uBit.power.powerDownDisable();

    deepsleep_test_send_time( "deepsleep_test5_onTimer1\n");

    deepsleep_test_zeroone();
    uBit.sleep(500);
    deepsleep_test_zeroone();

    uBit.power.powerDownEnable();

    // Request deep sleep and exit the handler
    uBit.power.deepSleepAsync();
}


void deepsleep_test5_onTimer2(MicroBitEvent e)
{
    uBit.power.powerDownDisable();

    deepsleep_test_send_time( "deepsleep_test5_onTimer2\n");

    deepsleep_test_threefour();
    uBit.sleep(500);
    deepsleep_test_threefour();

    uBit.power.powerDownEnable();

    // Request deep sleep and exit the handler
    uBit.power.deepSleepAsync();
}


void deepsleep_test5()
{
    deepsleep_test_send_time( "deepsleep_test5\n");

    // Show that display is off during sleep
    uBit.display.image.setPixelValue( 2, 0, 128);

    uint16_t        timer_id      = 60000;
    uint16_t        timer_value   = 1;

    uBit.messageBus.listen( timer_id, timer_value,     deepsleep_test5_onTimer1);
    uBit.messageBus.listen( timer_id, timer_value + 1, deepsleep_test5_onTimer2);

    // CODAL_TIMER_EVENT_FLAGS_WAKEUP makes the timer event trigger power up
    system_timer_event_every( 6000,  timer_id, timer_value,     CODAL_TIMER_EVENT_FLAGS_WAKEUP);
    system_timer_event_every( 20000, timer_id, timer_value + 1, CODAL_TIMER_EVENT_FLAGS_WAKEUP);

    uBit.power.deepSleepAsync();
}

////////////////////////////////////////////////////////////////
// HELPERS

void deepsleep_test_togglePixel( int x, int y)
{
    uBit.display.image.setPixelValue( x, y, uBit.display.image.getPixelValue( x, y) ? 0 : 255);
}


void deepsleep_test_sirenPixels( int x0, int y0, int x1, int y1, int ms)
{
    deepsleep_test_togglePixel( x0, y0);
    uBit.sleep(ms);
    deepsleep_test_togglePixel( x0, y0);
    deepsleep_test_togglePixel( x1, y1);
    uBit.sleep(ms);
    deepsleep_test_togglePixel( x1, y1);
}


void deepsleep_test_zeroone()
{
    deepsleep_test_send_time( "deepsleep_test_zeroone\n");
    deepsleep_test_sirenPixels( 0, 0, 1, 0, 150);
}


void deepsleep_test_threefour()
{
    deepsleep_test_send_time( "deepsleep_test_threefour\n");
    deepsleep_test_sirenPixels( 3, 0, 4, 0, 500);
}


void deepsleep_test_send_time( const char *suffix)
{
    uint64_t second = 1000000;

    uint64_t now = system_timer_current_time_us();

    int u  = (int) (now % second);
    int s = (int) (now / second);
    int m = s / 60;
    s = s % 60;
    int h  = m / 60;
    m = m % 60;

    ManagedString sh( h);
    ManagedString sm( m);  sm = "00"     + sm; sm = sm.substring( sm.length() - 2, 2);
    ManagedString ss( s);  ss = "00"     + ss; ss = ss.substring( ss.length() - 2, 2);
    ManagedString su( u);  su = "000000" + su; su = su.substring( su.length() - 6, 6);
    ManagedString msg = sh + ":" + sm + ":" + ss + "." + su + " " + ManagedString(suffix);
    uBit.serial.send( msg);
}

