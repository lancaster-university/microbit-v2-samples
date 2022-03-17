#include "MicroBit.h"
#include "SerialStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "MemorySource.h"
#include "DataStream.h"
#include "FIFOStream.h"

extern MicroBit uBit;

#define SAMPLE_FREQ 11000

MemorySource * player;
FIFOStream * fifo;
SerialStreamer * streamer;

void rec_simple_recorder()
{
    DMESG( "Configuring..." );    

    uBit.audio.soundExpressions.play("giggle");

    fifo = new FIFOStream( *uBit.audio.splitter );
    uBit.audio.mixer.addChannel(*fifo, SAMPLE_FREQ); 
    //streamer = new SerialStreamer( *fifo, SERIAL_STREAM_MODE_HEX );

    //uBit.audio.requestActivation();
    //uBit.audio.setSpeakerEnabled( true );
    //uBit.audio.setVolume( 200 );

    while( true )
    {
        if( uBit.buttonA.isPressed() )
        {
            fifo->setInputEnable( true );

            while( uBit.buttonA.isPressed() )
            {
                uBit.sleep( 100 );
            }

            fifo->setInputEnable( false );
            DMESG( "FIFO Length = %d", fifo->length() );
        }

        // Tap to play back
        if( uBit.buttonB.isPressed() )
        {
            fifo->setOutputEnable( true );

            while( uBit.buttonB.isPressed() )
            {
                uBit.sleep(100);
            }

            fifo->setOutputEnable( false );
            DMESG( "FIFO Length = %d", fifo->length() );
        }
        
        uBit.sleep( 100 );
    }
}

void rec_test_main()
{
    rec_simple_recorder();
}