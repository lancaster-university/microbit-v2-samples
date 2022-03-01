#include "TapeDeck.h"
#include "ErrorNo.h"
#include "DataStream.h"
#include "ManagedBuffer.h"
#include "CodalDmesg.h"

using namespace codal;

#define VERBOSITY 1

#if VERBOSITY == 1
    #define FMESG(...) DMESGF( __VA_ARGS__ )
#elif VERBOSITY == 2
    #define FMESG(...) DMESGF( "%s", __PRETTY_FUNCTION__ ); DMESG( __VA_ARGS__ )
#elif VERBOSITY == 3
    #define FMESG(...) DMESGF( "%s:%d", __PRETTY_FUNCTION__, __LINE__ ); DMESG( __VA_ARGS__ )
#elif VERBOSITY == 4
    #define FMESG(...) DMESGF( "%s:%d - %s", __FILE__, __LINE__, __PRETTY_FUNCTION__ ); DMESG( __VA_ARGS__ )
#else
    #define FMESG(...)
#endif

TapeDeck::TapeDeck( DataSource &upstream )
{
    this->bufferCount = 0;
    this->bufferLength = 0;
    
    this->downStream = NULL;
    this->upStream = &upstream;
}

TapeDeck::~TapeDeck()
{
    //
}

bool TapeDeck::canPull()
{
    return this->bufferLength > 0;
}

ManagedBuffer TapeDeck::pull()
{
    ManagedBuffer out = buffer[0];

    if( this->bufferLength > 0 )
    {
        for (int i = 0; i < TAPEDECK_MAXIMUM_BUFFERS-1; i++)
            buffer[i] = buffer[i + 1];
        buffer[TAPEDECK_MAXIMUM_BUFFERS-1] = ManagedBuffer();

        this->bufferLength -= out.length();
        this->bufferCount--;
    }
    
    return out;
}

int TapeDeck::length()
{
    return this->bufferLength;
}

bool TapeDeck::isFull() {

    return this->bufferCount < TAPEDECK_MAXIMUM_BUFFERS;
}

void TapeDeck::dumpState()
{
    FMESG(
        "TapeDeck { bufferCount = %d/%d, bufferLength = %dB }",
        this->bufferCount,
        TAPEDECK_MAXIMUM_BUFFERS,
        this->bufferLength
    );
}

int TapeDeck::pullRequest()
{
    if( this->bufferCount >= TAPEDECK_MAXIMUM_BUFFERS )
        return DEVICE_NO_RESOURCES;

    ManagedBuffer inBuffer = this->upStream->pull();

    this->buffer[ this->bufferCount++ ] = inBuffer;
    this->bufferLength += inBuffer.length();

    return DEVICE_OK;
}

void TapeDeck::connect( DataSink &sink )
{
    this->downStream = &sink;
}

void TapeDeck::disconnect()
{
    this->downStream = NULL;
}

int TapeDeck::getFormat()
{
    return upStream->getFormat();
}

int TapeDeck::setFormat( int format )
{
    FMESG( "Ignored setFormat, unsupported operation." );
    return 0;
}