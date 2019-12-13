#include "MicroBit.h"
#include "NRF52Microphone.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "Tests.h"

NRF52Microphone *mic = NULL;
SerialStreamer *streamer = NULL;
LevelDetector *level = NULL;
StreamNormalizer *normalizer = NULL;
int claps = 0;

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

void init_streaming()
{
    if (mic == NULL)
        mic = new NRF52Microphone(uBit.io.microphone, 11000);

    if (streamer == NULL)
        streamer = new SerialStreamer(mic->output);
}

void init_clap_detect()
{
    claps = -1;

    DMESG("INIT_MIC:"); uBit.sleep(100);
    if (mic == NULL)
        mic = new NRF52Microphone(uBit.io.microphone, 11000);
    DMESG("INIT_MIC: DONE"); uBit.sleep(100);

    DMESG("INIT_NORM:"); uBit.sleep(100);
    if (normalizer == NULL)
        normalizer = new StreamNormalizer(mic->output, 4096);
    DMESG("INIT_NORM: DONE"); uBit.sleep(100);

    DMESG("INIT_DETECTOR:"); uBit.sleep(100);
    if (level == NULL)
        level = new LevelDetector(normalizer->output, 2500, 2000);
    DMESG("INIT_DETECTOR: DONE"); uBit.sleep(100);
}


void
mems_mic_test()
{
    init_streaming();

    // Enable RUN_MIC
    uBit.io.runmic.setDigitalValue(1);

    streamer->setDivisor(8);
    mic->setGain(4);
    mic->enable();

    while(1)
        uBit.sleep(1000);
}

void
piezo_mic_test()
{
    init_streaming();

    // Ensure the other side of the piezo is high impedance
    uBit.io.speaker.setPull(PullNone);
    uBit.io.speaker.getDigitalValue();

    // Enable RUN_MIC
    uBit.io.runmic.setDigitalValue(1);

    streamer->setDivisor(64);
    mic->setGain(0);
    mic->enable();

    while(1)
        uBit.sleep(1000);
}

void
piezo_clap_test(int wait_for_clap)
{
    DMESG("PIEZO_INIT"); uBit.sleep(100);
    init_clap_detect();
    DMESG("PIEZO_INIT COMPLETE"); uBit.sleep(100);

    // Ensure the other side of the piezo is high impedance
    uBit.io.speaker.setPull(PullNone);
    uBit.io.speaker.getDigitalValue();

    // Enable RUN_MIC
    uBit.io.runmic.setDigitalValue(1);

    normalizer->setGain(4096);
    mic->setGain(0);
    mic->enable();

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
    {
        uBit.sleep(1000);
        DMESG(".");
    }

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

void
mems_clap_test(int wait_for_clap)
{
    init_clap_detect();

    // Enable RUN_MIC
    uBit.io.runmic.setDigitalValue(1);

    normalizer->setGain(32768);
    mic->setGain(0);
    mic->enable();

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

