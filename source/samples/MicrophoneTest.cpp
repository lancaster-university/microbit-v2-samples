#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"
#include "StreamRecording.h"
#include "Tests.h"

static NRF52ADCChannel *mic = NULL;
static SerialStreamer *streamer = NULL;
static StreamNormalizer *processor = NULL;
static LevelDetector *level = NULL;
static LevelDetectorSPL *levelSPL = NULL;
static StreamRecording *recording = NULL;
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

void hBar( int value, int max )
{
    DMESGN( "[" );
    for( int i=0; i<max; i++ ) {
        if( i<value )
            DMESGN( "=" );
        else
            DMESGN( " " );
    }
    DMESGN( "] %d/%d\r", value, max );
}


// Note - this needs cleaning up before release! -JV.
static SplitterChannel * inChannel = NULL;
void
mems_record_playback_test()
{
    if( recording == NULL ) {
        inChannel = uBit.audio.splitter->createChannel();
        recording = new StreamRecording( *inChannel );

        MixerChannel * channel = uBit.audio.mixer.addChannel( *recording, 22000 );

        channel->setVolume( 100.0 );
        uBit.audio.mixer.setVolume( 1000 );
        uBit.audio.setSpeakerEnabled( true );

        DMESG(
            "mic(%d Hz) -> processor(%d Hz) -> inChannel(%d Hz)",
            (int)uBit.audio.mic->getSampleRate(),
            (int)uBit.audio.processor->getSampleRate(),
            (int)inChannel->getSampleRate()
        );

        //uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
        //uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
    }

    uBit.display.scroll( "Rec" );

    while(1)
    {
        if( uBit.buttonA.wasPressed() )
        {
            inChannel->requestSampleRate( 22000 );
            uBit.display.printChar( 'R' );
            recording->record();

            // Record until we run out of buffer, or we have 2 seconds of audio
            while( recording->isRecording() )
            {
                uBit.sleep( 200 );
            }
            uBit.display.clear();

            recording->stop();
        }

        if( uBit.buttonB.wasPressed() )
        {
            uBit.display.printChar( 'P' );

            MicroBitAudio::requestActivation();
            recording->play();
            while( recording->isPlaying() )
            {
                uBit.sleep( 200 );
            }
            uBit.display.clear();
        }

        if( uBit.logo.isPressed() )
        {
            int val = (int)uBit.audio.levelSPL->getValue() - 50;

            for( int y=0; y<5; y++ )
                for( int x=0; x<5; x++ )
                    uBit.display.image.setPixelValue( x, 4-y, (y*10) < val ? 255 : 0 );

            hBar( val, 50 );
        }
        else {
            uBit.display.clear();
        }

        /*if( uBit.buttonA.isPressed() )
            inChannel->requestSampleRate( inChannel->getSampleRate() + 1000 );
        
        if( uBit.buttonB.isPressed() )
            inChannel->requestSampleRate( inChannel->getSampleRate() - 1000 );

        DMESG(
            "(%d) -> (%d) -> (%d)",
            (int)uBit.audio.mic->getSampleRate(),
            (int)uBit.audio.processor->getSampleRate(),
            (int)inChannel->getSampleRate()
        );*/

        uBit.sleep( 1 );
    }
    //uBit.audio.splitter
}