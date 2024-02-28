/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "Tests.h"

int last_t0, last_t1, last_t2;
static int c0 = 0;
static int c1 = 0;
static int c2 = 0;

#define TOUCH_SENSE_CLUSTER_LOW                     0
#define TOUCH_SENSE_CLUSTER_HIGH                    1
#define TOUCH_SENSE_INITIALISATION_COUNT            10
#define TOUCH_SENSE_CONVERGENCE_RATE                0.98f
#define TOUCH_SENSE_SMOOTHING_RATE                  0.5f
#define TOUCH_SENSE_CLUSTER_CEILING                 10000

int loops = 0;

float threshold;
int currentCluster = TOUCH_SENSE_CLUSTER_LOW;
int initializing;

float lo_mean;
float hi_mean;
float touch_level;

void
calibrateInit()
{
    threshold = 500.0f;
    currentCluster = TOUCH_SENSE_CLUSTER_LOW;
    initializing = TOUCH_SENSE_INITIALISATION_COUNT;
    lo_mean = 0.0f;
    hi_mean = 0.0f;
    touch_level = 0.0f;
}

void 
calibrateTest(float sample)
{
    if (initializing)
    {
        lo_mean += sample;
        initializing--;

        if (initializing == 0)
        {
            lo_mean = lo_mean / TOUCH_SENSE_INITIALISATION_COUNT;
            hi_mean = lo_mean + threshold * 2;
            touch_level = lo_mean;
        }
        return;
    }

    touch_level = touch_level * TOUCH_SENSE_SMOOTHING_RATE + sample * (1-TOUCH_SENSE_SMOOTHING_RATE);

    if (currentCluster == TOUCH_SENSE_CLUSTER_LOW && touch_level < TOUCH_SENSE_CLUSTER_CEILING && touch_level < hi_mean - threshold)
        lo_mean = lo_mean * TOUCH_SENSE_CONVERGENCE_RATE + touch_level * (1-TOUCH_SENSE_CONVERGENCE_RATE);

    if (currentCluster == TOUCH_SENSE_CLUSTER_HIGH && touch_level < TOUCH_SENSE_CLUSTER_CEILING && touch_level > lo_mean + threshold)
        hi_mean = hi_mean * TOUCH_SENSE_CONVERGENCE_RATE + touch_level * (1-TOUCH_SENSE_CONVERGENCE_RATE);

    if (touch_level < lo_mean + threshold)
        currentCluster = TOUCH_SENSE_CLUSTER_LOW;

    if (touch_level > hi_mean - threshold)
        currentCluster = TOUCH_SENSE_CLUSTER_HIGH;
    
    loops++;

    if (loops == 10){
        DMESG("%d: [SAMPLE: %d][LEVEL: %d][LO_MEAN: %d] [HI_MEAN: %d]", currentCluster, (int)sample, (int)touch_level, (int)lo_mean, (int)hi_mean);
        loops = 0;
    }

    //DMESG("[SAMPLE: %d]", (int)sample);
}

static void
onCalibrate(MicroBitEvent)
{
    
    uBit.serial.printf("--- CALIBRATE ---\n");
    c0 = last_t0;
    c1 = last_t1;
    c2 = last_t2;
}

static void
onPrint(MicroBitEvent)
{
    while(1)
        uBit.display.scroll(last_t2);    
}

static void onTouchP0(MicroBitEvent e)
{
    DMESG("TOUCH: P0");
}
static void onTouchP1(MicroBitEvent e)
{
    DMESG("TOUCH: P1");
}
static void onTouchP2(MicroBitEvent e)
{
    DMESG("TOUCH: P2");
}
static void onTouchLogo(MicroBitEvent e)
{
    DMESG("TOUCH: LOGO");
}

void 
cap_touch_test()
{
    uBit.messageBus.listen(MICROBIT_ID_IO_P0, MICROBIT_BUTTON_EVT_CLICK, onTouchP0);
    uBit.messageBus.listen(MICROBIT_ID_IO_P1, MICROBIT_BUTTON_EVT_CLICK, onTouchP1);
    uBit.messageBus.listen(MICROBIT_ID_IO_P2, MICROBIT_BUTTON_EVT_CLICK, onTouchP2);
    uBit.messageBus.listen(MICROBIT_ID_LOGO, MICROBIT_BUTTON_EVT_CLICK, onTouchLogo);

    while(1)
    {
        uBit.display.image.setPixelValue(0,0,uBit.io.P0.isTouched(TouchMode::Resistive) ? 255 : 0);
        uBit.display.image.setPixelValue(2,0,uBit.io.P1.isTouched(TouchMode::Resistive) ? 255 : 0);
        uBit.display.image.setPixelValue(4,0,uBit.io.P2.isTouched(TouchMode::Resistive) ? 255 : 0);

        uBit.display.image.setPixelValue(2,4,uBit.logo.isPressed() ? 255 : 0);

        // Only useful is pins are placed in capacitative mode...
        if (uBit.buttonA.isPressed())
        {
            uBit.io.P0.touchCalibrate();
            uBit.io.P1.touchCalibrate();
            uBit.io.P2.touchCalibrate();            

            uBit.sleep(1000);
        }

        uBit.sleep(100);
    }
}

void 
cap_touch_test_raw()
{
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onCalibrate);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onPrint);

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.

    int t, t0, t1, t2;
    //int THRESHOLD = 100;
    int T_MAX = 2000;

    while(1)
    {
        // Discharge each pin
        uBit.io.P0.setDigitalValue(0);
        uBit.io.P1.setDigitalValue(0);
        uBit.io.P2.setDigitalValue(0);

        uBit.sleep(20);

        // Convert each pin to an input, and time how long it takes to rise to a CMOS logic HI.
        t=1; 
        t0=0;
        t1=0;
        t2=0;

        uBit.io.P0.getDigitalValue(PullMode::None);
        uBit.io.P1.getDigitalValue(PullMode::None);
        uBit.io.P2.getDigitalValue(PullMode::None);

        while (t < T_MAX)
        {
            if (t0 == 0 && uBit.io.P0.getDigitalValue())
                t0=t;

            if (t1 == 0 && uBit.io.P1.getDigitalValue())
                t1=t;

            if (t2 == 0 && uBit.io.P2.getDigitalValue())
                t2=t;

            t++;
        }

        if(t0 == 0)
            t0 = T_MAX;

        if(t1 == 0)
            t1 = T_MAX;

        if(t2 == 0)
            t2 = T_MAX;

        last_t0 = t0;
        last_t1 = t1;
        last_t2 = t2;

        // apply calibration
        t0 = t0 - c0;
        t1 = t1 - c1;
        t2 = t2 - c2;

/*
        if (t0 > THRESHOLD)
            uBit.display.image.setPixelValue(0,0,255);
        else
            uBit.display.image.setPixelValue(0,0,0);

        if (t1 > THRESHOLD)
            uBit.display.image.setPixelValue(2,0,255);
        else
            uBit.display.image.setPixelValue(2,0,0);

        if (t2 > THRESHOLD)
            uBit.display.image.setPixelValue(4,0,255);
        else
            uBit.display.image.setPixelValue(4,0,0);
*/
        uBit.serial.printf("[P0: %d] [P1: %d] [P2: %d]\n", t0, t1, t2);
    }
    

    release_fiber();
}

