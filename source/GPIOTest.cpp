#include "MicroBit.h"
#include "Tests.h"
#include "NRF52PWM.h"
#include "MemorySource.h"

Pin *edgeConnector[] = {
    &uBit.io.P0, 
    &uBit.io.P1, 
    &uBit.io.P2, 
    &uBit.io.P3, 
    &uBit.io.P4, 
    &uBit.io.P5, 
    &uBit.io.P6, 
    &uBit.io.P7, 
    &uBit.io.P8, 
    &uBit.io.P9, 
    &uBit.io.P10, 
    &uBit.io.P11, 
    &uBit.io.P12, 
    &uBit.io.P13, 
    &uBit.io.P14, 
    &uBit.io.P15, 
    &uBit.io.P16, 
    &uBit.io.P19, 
    &uBit.io.P20
};

//Pin *analogPins[] = {&uBit.io.P1, &uBit.io.P2};
static Pin *analogPins[] = {&uBit.io.P1};
static NRF52PWM *pwm = NULL;
static MemorySource *pwmSource = NULL;

static uint16_t square[4];

void
edge_connector_test()
{
    uBit.display.disable();
    uBit.buttonA.disable();
    uBit.buttonB.disable();

    for (Pin *p : edgeConnector)
        p->setDigitalValue(0);

    while(1)
    {
        for (Pin *p : edgeConnector)
        {
            p->setDigitalValue(1);
            uBit.sleep(500);
            p->setDigitalValue(0);
            uBit.sleep(500);
        }
    }
}

void
analog_test()
{
    Image level(5,5);

    while(1)
    {
        int x=0;
        int y=0; 
        int px = analogPins[0]->getAnalogValue() / 40;

        level.clear();

        while(px)
        {
            level.setPixelValue(x,y, 255);
            x = (x+1) % 5;
            if (x == 0)
                y = (y+1) % 5;

            px--;
        }

        uBit.display.print(level);

        for (Pin *p : analogPins)
        {
            int v = p->getAnalogValue();
            DMESG("%d", v);
        }

        DMESG("----");
        uBit.sleep(100);
    }
}
void
gpio_test()
{
    int value = 0;

    uBit.io.P2.getDigitalValue();
    uBit.io.P2.setPull(PullMode::Up);

    while(1)
    {
        if (uBit.io.P2.getDigitalValue())
            uBit.display.image.setPixelValue(0,0,255);
        else
            uBit.display.image.setPixelValue(0,0,0);

        uBit.io.P1.setDigitalValue(value);
        value = !value;

        uBit.sleep(100);
    }
}

void
highDriveTest()
{
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
}

void
pwm_pin_test()
{
    int freq = 200;
    uBit.display.print('c');
    uBit.sleep(500);
    DMESG("PWM_PIN_TEST: STARTING...");
    uBit.io.speaker.setAnalogValue(512);
    DMESG("PWM_PIN_TEST: setAnalogValue...");

    while(1)
    {
        uBit.io.speaker.setAnalogPeriodUs(1000000/freq);
        DMESG("PWM_PIN_TEST: setAnalogPeriodUs...");

        uBit.sleep(1000);
        uBit.io.speaker.setAnalogValue(0);
        uBit.sleep(1000);
        uBit.io.speaker.setAnalogValue(512);
        
        freq += 100;
    }
}

void
pwm_test()
{
    DMESG("PWM TEST: STARTING...");

    if (pwmSource == NULL)
        pwmSource = new MemorySource();

    if (pwm == NULL)
        pwm = new NRF52PWM(NRF_PWM0, pwmSource->output, 200);
   
    uBit.io.speaker.setHighDrive(true);

    pwm->connectPin(uBit.io.speaker, 0);
    pwm->connectPin(uBit.io.P0, 1);

    DMESG("SPEAKER TEST: WOBBLING... [max: %d]", pwm->getSampleRange());

    int freq = 200;

    while(1)
    {
        pwm->setSampleRate(freq);
        for (int i=0; i<4; i++)
            square[i] = pwm->getSampleRange()/2;

        pwmSource->play(square, 4, 0); 

        DMESG("SPEAKER TEST: %d Hz", freq);
        uBit.sleep(3000);

        freq = freq + 100;
    }

    // Should never get here...
    DMESG("SPEAKER TEST: EXITING...");
}
