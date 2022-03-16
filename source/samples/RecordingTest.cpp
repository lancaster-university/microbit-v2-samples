#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "MemorySource.h"
#include "DataStream.h"
#include "FIFOStream.h"

extern MicroBit uBit;

#define SAMPLE_FREQ 110000
#define SAMPLE_HZ_TO_USEC(hz) (1e6 / (hz))

MemorySource * player;
FIFOStream * fifo;
SerialStreamer * streamer;

void rec_simple_recorder()
{
    DMESG( "Configuring..." );    

    player = new MemorySource();
    player->setFormat( uBit.audio.mic->getFormat() );
    uBit.audio.mixer.addChannel(
        *player,
        player->getFormat() // Copy whatever format the mic is currently using
    );

    uBit.audio.mixer.setSampleRate( SAMPLE_FREQ );
    uBit.adc.setSamplePeriod( SAMPLE_HZ_TO_USEC(SAMPLE_FREQ) );

    fifo = new FIFOStream( *uBit.audio.mic );
    fifo->setInputEnable( true );
    fifo->setOutputEnable( true );

    streamer = new SerialStreamer( *fifo, SERIAL_STREAM_MODE_HEX );

    while( true )
    {
        if( uBit.buttonA.isPressed() )
        {
            uBit.audio.activateMic();

            while( uBit.buttonA.isPressed() )
            {
                fifo->pullRequest();
                //uint16_t sample = uBit.audio.mic->getSample();
                //DMESGF( "Val = %d", sample );
            }
            DMESG( "Length = %d", fifo->length() );

            uBit.audio.deactivateMic();
        }

        // Tap to play back
        if( uBit.buttonB.isPressed() )
        {
            uBit.audio.requestActivation();
            uBit.audio.setSpeakerEnabled( true );
            uBit.audio.setVolume( 200 );
            while( uBit.buttonB.isPressed() )
            {
                if( fifo->canPull() )
                    player->play( fifo->pull(), 1 );
            }
            DMESG( "Length = %d", fifo->length() );
            uBit.audio.setSpeakerEnabled( false );
        }
        
        uBit.sleep( 100 );
    }
}

void rec_test_main()
{
    rec_simple_recorder();
}