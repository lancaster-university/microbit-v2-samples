#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "Tests.h"

static NRF52ADCChannel *mic = NULL;
static SerialStreamer *streamer = NULL;
static StreamNormalizer *processor = NULL;
static LevelDetector *level = NULL;
static int claps = 0;
static volatile int sample;

void
onLoud(MicroBitEvent)
{
    DMESG("LOUD");
    claps++;
    if (claps >= 10)
        claps = 0;

    uBit.display.print(claps);
}

void
onQuiet(MicroBitEvent)
{
}

void mems_mic_drift_test()
{
    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    while(true)
    {
        sample = uBit.io.P0.getAnalogValue();
        uBit.sleep(250);

        sample = uBit.io.microphone.getAnalogValue();
        uBit.sleep(250);

        uBit.display.scroll(sample);
    }
}

void
mems_mic_test()
{
    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);          // Uncomment for v1.47.2
        //mic->setGain(7,1);        // Uncomment for v1.46.2
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 0.05f, true, DATASTREAM_FORMAT_8BIT_SIGNED);

    if (streamer == NULL)
        streamer = new SerialStreamer(processor->output, SERIAL_STREAM_MODE_BINARY);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    while(1)
        uBit.sleep(1000);
}

void
mems_clap_test(int wait_for_clap)
{
    claps = -1;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,1);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true);

    if (level == NULL)
        level = new LevelDetector(processor->output, 600, 200);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}





