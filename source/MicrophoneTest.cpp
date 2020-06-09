#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "Tests.h"

NRF52ADCChannel *mic = NULL;
SerialStreamer *streamer = NULL;
LevelDetector *level = NULL;
StreamNormalizer *processor = NULL;
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
    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,1);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 0.05f, true, DATASTREAM_FORMAT_8BIT_SIGNED);

    if (streamer == NULL)
        streamer = new SerialStreamer(processor->output, SERIAL_STREAM_MODE_BINARY);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);
}

void init_clap_detect()
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
}


void
mems_mic_test()
{
    init_streaming();

    while(1)
        uBit.sleep(1000);
}

void
mems_clap_test(int wait_for_clap)
{
    init_clap_detect();

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

