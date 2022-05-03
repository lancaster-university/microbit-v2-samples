#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "MemorySource.h"
#include "DataStream.h"
#include "FIFOStream.h"
#include "StreamFlowTrigger.h"
#include "StreamRecording.h"
#include "StreamFilter.h"

#define CHANNEL_FREQ 22000

extern MicroBit uBit;

MemorySource *     player    = NULL;
SerialStreamer *   streamer  = NULL;
StreamRecording *  recording = NULL;
StreamFilter *     filter    = NULL;
StreamNormalizer * processor = NULL;
LevelDetector *    level     = NULL;

float clamp( float val, float min, float max ) {
    if( val < min )
        return min;
    if( val > max )
        return max;
    return val;
}

void fill_display( unsigned int state )
{
    uBit.display.setDisplayMode( DISPLAY_MODE_BLACK_AND_WHITE );
}

void rec_simple_recorder()
{
    DMESG( "Configuring..." );
    bool squeaky = false;

    MicroBitAudio::requestActivation();

    filter = new StreamFilter( *uBit.audio.splitter );
    filter->lpf_beta = 0.030f;

    recording = new StreamRecording( *filter );
    
    MixerChannel * channel = uBit.audio.mixer.addChannel( *recording, CHANNEL_FREQ );

    // By connecting to the mic channel, we activate it automatically, so shut it down again.
    uBit.audio.deactivateMic();
    uBit.audio.mic->disable();

    channel->setVolume( 100.0 );
    //uBit.audio.processor->setGain( 0.5 );
    //uBit.audio.mic->setGain( 7, 0 );
    uBit.audio.mixer.setVolume( 1000 );
    uBit.audio.setSpeakerEnabled( true );

    uBit.display.scroll( ManagedString("Ready!"), 60 );

    while( true )
    {
        uBit.display.clear();
        uBit.audio.mixer.setSampleRate( 44000 );

        if( uBit.buttonA.isPressed() )
        {
            DMESG( "Recording!" );
            uBit.display.printChar( 'R' );

            uBit.audio.mic->enable();
            uBit.audio.activateMic();
            recording->record();

            while( uBit.buttonA.isPressed() && recording->isRecording() )
            {
                if( uBit.timer.getTime() % 1000 > 500 )
                    uBit.display.printChar( 'R' );
                else
                    uBit.display.printChar( '>' );
                uBit.sleep( 100 );
            }

            recording->stop();
            uBit.audio.deactivateMic();
            uBit.audio.mic->disable();

            DMESG( "Stopped!" );
            uBit.display.printChar( 'X' );

            // Catch cases where we run out memory, but the button is still held. This is just to print the 'Stopped' output, the
            // actual recording API doesn't care, and will just stop recording when its full.
            while( uBit.buttonA.isPressed())
                uBit.sleep( 100 );
        }

        if( uBit.logo.isPressed() )
        {
            while( uBit.logo.isPressed() ) {

                if( uBit.buttonA.isPressed() )
                {
                    //filter->avg_enabled = !filter->avg_enabled;
                    squeaky = !squeaky;

                    if( squeaky )
                        uBit.display.scroll( ManagedString("Gyro: ON"), 60 );
                    else
                        uBit.display.scroll( ManagedString("Gyro: OFF"), 60 );

                    DMESG( "Gyro control: %d", squeaky );
                    uBit.sleep( 250 );
                }

                if( uBit.buttonB.isPressed() )
                {
                    filter->lpf_enabled = !filter->lpf_enabled;

                    if( filter->lpf_enabled )
                        uBit.display.scroll( ManagedString("Filter: ON"), 60 );
                    else
                        uBit.display.scroll( ManagedString("Filter: OFF"), 60 );
                    
                    DMESG( "LPF state: %d", filter->lpf_enabled );
                    uBit.sleep( 250 );
                }

                uBit.sleep( 5 );
            }
        }

        // Tap to play back
        if( uBit.buttonB.isPressed() )
        {
            uBit.display.printChar( 'P' );

            recording->play();
            while( recording->isPlaying() ) //uBit.buttonB.isPressed() )
            {
                if( squeaky )
                {
                    float uFreq = CHANNEL_FREQ + (uBit.accelerometer.getPitch() * 1000.0f);
                    uFreq = clamp( uFreq, 1000, 1e6 );
                    uBit.audio.mixer.setSampleRate( uFreq );
                }

                if( uBit.timer.getTime() % 1000 > 500 )
                    uBit.display.printChar( 'P' );
                else
                    uBit.display.printChar( '>' );
                uBit.sleep( 5 );
            }
            recording->stop();

            uBit.display.printChar( 'X' );
        }
    }
}

void mic_power_test()
{
    MicroBitAudio::requestActivation();

    recording = new StreamRecording( *uBit.audio.splitter );

    while( true )
    {
        DMESG( "Enabling the audio pipeline" );
        //uBit.audio.enable();
        uBit.audio.mic->enable();
        uBit.audio.activateMic();
        
        DMESG( "Recording until full..." );
        recording->record();
        while( recording->isRecording() ) {
            DMESG( "Length = %d", recording->length() );
            uBit.sleep( 100 );
        }

        DMESG( "Disabling the audio pipeline" );
        uBit.audio.deactivateMic();
        uBit.audio.mic->disable();
        //uBit.audio.disable();

        DMESG( "Erase recording..." );
        recording->erase();
        uBit.sleep( 1000 );
    }
}

void startRecordingEvt(MicroBitEvent) {
    DMESG( "Recording!" );
    uBit.display.printChar( 'R' );

    recording->record();
    while( recording->isRecording() )
    {
        if( uBit.timer.getTime() % 1000 > 500 )
            uBit.display.printChar( 'R' );
        else
            uBit.display.printChar( '>' );
        uBit.sleep( 100 );
    }
    recording->stop();

    DMESG( "Stopped!" );
    uBit.display.clear();

    recording->play();
    while( recording->isPlaying() )
    {
        if( uBit.timer.getTime() % 1000 > 500 )
            uBit.display.printChar( 'P' );
        else
            uBit.display.printChar( '>' );
        uBit.sleep( 100 );
    }
    recording->erase();
    uBit.display.clear();

    uBit.sleep( 1000 );
}

void automatic_recorder()
{
    MicroBitAudio::requestActivation();

    filter = new StreamFilter( *uBit.audio.splitter );
    filter->lpf_beta = 0.030f;
    filter->lpf_enabled = true;

    recording = new StreamRecording( *filter );
    MixerChannel * channel = uBit.audio.mixer.addChannel( *recording, CHANNEL_FREQ );
    
    processor = new StreamNormalizer(
                        *uBit.audio.splitter,
                        1.0f,
                        true,
                        DATASTREAM_FORMAT_UNKNOWN,
                        10 );
    level = new LevelDetector( processor->output, 1, 0 );

    uBit.messageBus.listen( DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, startRecordingEvt );

    // By connecting to the mic channel, we activate it automatically, so shut it down again.
    //uBit.audio.deactivateMic();
    //uBit.audio.mic->disable();

    channel->setVolume( 100.0 );
    uBit.audio.mic->setGain( 5, 3 );
    uBit.audio.mixer.setVolume( 1000 );
    uBit.audio.setSpeakerEnabled( true );

    uBit.sleep( 1000 );

    while( true )
    {
        int value = level->getValue();
        if( value > 0 )
            DMESG( "Level: %d", value );
        uBit.sleep( 100 );
    }
}

void rec_test_main()
{
    //automatic_recorder();
    rec_simple_recorder();
    //mic_power_test();
}