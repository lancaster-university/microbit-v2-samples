#include <stdio.h>
#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"
#include "Tests.h"

static NRF52ADCChannel *mic = NULL;
static SerialStreamer *streamer = NULL;
static StreamNormalizer *processor = NULL;
static LevelDetector *level = NULL;
static LevelDetectorSPL *levelSPL = NULL;
static int claps = 0;
static volatile int sample;

static void
onLoud(MicroBitEvent)
{
    DMESG("LOUD");
    claps++;
    if (claps >= 10)
        claps = 0;

    uBit.display.print(claps);
}

static void
onQuiet(MicroBitEvent)
{
    DMESG("QUIET");
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

// WARNING! For this test to run correctly floats for printf/sprintf/snprintf
// have to be enabled by adding this flag to the linker (target.json):
// -u _printf_float
void
mems_mic_zero_offset_test()
{
    LevelDetectorSPL* levelSPL = new LevelDetectorSPL(uBit.audio.processor->output, 85.0, 65.0, 16.0, 0, DEVICE_ID_SYSTEM_LEVEL_DETECTOR, false);
    uBit.audio.activateMic();

    char float_str[20];
    volatile auto value = 0;

    while (true) {
        value = levelSPL->getValue();
        snprintf(float_str, 80, "%.4f", uBit.audio.processor->zeroOffset);
        uBit.serial.printf("%s\n", float_str);
        uBit.sleep(1);
    }
}

void
mems_clap_test(int wait_for_clap)
{
    claps = 0;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    if (level == NULL)
        level = new LevelDetector(processor->output, 150, 75);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

void
mems_clap_test_spl(int wait_for_clap)
{
    claps = 0;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    if (levelSPL == NULL)
        levelSPL = new LevelDetectorSPL(processor->output, 75.0, 60.0, 9, 52, DEVICE_ID_MICROPHONE);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);
}

class MakeCodeMicrophoneTemplate {
  public:
    MIC_DEVICE microphone;
    LevelDetectorSPL level;
    MakeCodeMicrophoneTemplate() MIC_INIT { MIC_ENABLE; }
};

void
mc_clap_test()
{
    new MakeCodeMicrophoneTemplate();

    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);

    while(1)
    {
        uBit.sleep(1000);
    }
}
