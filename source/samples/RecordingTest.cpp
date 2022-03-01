#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "MemorySource.h"
#include "DataStream.h"
#include "TapeDeck.h"

extern MicroBit uBit;

NRF52ADCChannel *mic = NULL;
SerialStreamer *streamer = NULL;
StreamNormalizer *processor = NULL;
LevelDetector *level = NULL;
MemorySource *player = NULL;

#define SAMPLE_FREQ (11e3)
#define SAMPLE_HZ_TO_USEC(hz) (1e6 / (hz))

void rec_simple_recorder()
{
    DMESG( "Configuring..." );

    if( mic == NULL )
    {
        mic = uBit.adc.getChannel( uBit.io.microphone );
        uBit.adc.setSamplePeriod( SAMPLE_HZ_TO_USEC( SAMPLE_FREQ ) ); // 11kHz
        mic->setGain( 7, 0 );
    }

    /*if( processor == NULL )
        processor = new StreamNormalizer( mic->output, 1.0f, true );
    
    if( level == NULL )
        level = new LevelDetector( processor->output, 300, 200 );*/

    if( player == NULL )
    {
        player = new MemorySource();
        player->setFormat( DATASTREAM_FORMAT_8BIT_UNSIGNED );
        uBit.audio.mixer.addChannel(
            *player,
            SAMPLE_FREQ // 11k
        );
    }

    TapeDeck tape( *mic );

    DMESG( "READY..." );

    while( true )
    {
        uBit.display.clear();

        // Hold to record
        if( uBit.buttonA.isPressed() )
        {
            DMESG( "REC:START" );
            uBit.audio.activateMic();
            while( uBit.buttonA.isPressed() )
            {
                tape.dumpState();
                tape.pullRequest();

                /*int value = recBuffer.pullRequest();
                if( value == DEVICE_OK ) {
                    DMESG( "OK" );
                }
                else if( value == DEVICE_NO_RESOURCES )
                {
                    DMESG( "OOM" );
                }*/

                //int32_t value = level->getValue();
                /*DMESG( "%d\t%d Bytes (full? %d)", value, recBuffer->length(), recBuffer->full() );

                uBit.display.clear();
                for( int i = 0; i < 5; i++ )
                {
                    if( value > (i * (i*5)) )
                    {
                        for( int x = 0; x < 5; x++ )
                        {
                            uBit.display.image.setPixelValue( x, 5 - i, 255 );
                        }
                    }
                }*/
            }
            uBit.audio.deactivateMic();

            DMESG( "REC:STOP" );
        }

        // Tap to play back
        if( uBit.buttonB.isPressed() )
        {
            DMESG( "PLAY ? %d Bytes", tape.length() );
            tape.dumpState();
            uBit.audio.setVolume( 255 );
            uBit.audio.setSpeakerEnabled( true );

            while( tape.length() > 0 )
            {
                ManagedBuffer block = tape.pull();
                DMESG( "len = %d, first = %d", block.length(), block.getByte(0) );
                player->play( block, 1 );
                tape.dumpState();

                uBit.sleep( 100 );
            }

            uBit.audio.setSpeakerEnabled( false );
            DMESG( "PLAY:STOP" );
        }
        
    }
    uBit.display.clear();
}

void rec_test_main()
{
    rec_simple_recorder();
}