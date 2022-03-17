#include "FIFOStream.h"
#include "ErrorNo.h"
#include "DataStream.h"
#include "ManagedBuffer.h"
#include "CodalDmesg.h"
#include "MessageBus.h"

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

FIFOStream::FIFOStream( DataSource &source ) : upStream( source )
{
    this->bufferCount = 0;
    this->bufferLength = 0;
    this->notifyID = -1;
    
    this->downStream = NULL;
    source.connect( *this );

    this->allowInput = false;
    this->allowOutput = false;

}

FIFOStream::~FIFOStream()
{
    //
}

bool FIFOStream::canPull()
{
    DMESG("FIFO:canPull");
    return (this->bufferLength > 0) && this->allowOutput;
}

ManagedBuffer FIFOStream::pull()
{
    DMESG("FIFO:pull");
    if( (this->bufferLength > 0) && this->allowOutput )
    {
        ManagedBuffer out = buffer[0];

        for (int i = 0; i < TAPEDECK_MAXIMUM_BUFFERS-1; i++)
            buffer[i] = buffer[i + 1];
        buffer[TAPEDECK_MAXIMUM_BUFFERS-1] = ManagedBuffer();

        this->bufferLength -= out.length();
        this->bufferCount--;

        this->downStream->pullRequest();
        return out;
    }

    // Note - this may be bogus as we technically don't send any data, we'd be relying on the
    // downstream to not bother actually pulling via canPull() or getting a null ManagedBuffer...
    this->downStream->pullRequest();
    return ManagedBuffer();
}

int FIFOStream::length()
{
    DMESG("FIFO:length");
    return this->bufferLength;
}

bool FIFOStream::isFull() {
    DMESG("FIFO:isFull");
    return this->bufferCount < TAPEDECK_MAXIMUM_BUFFERS;
}

void FIFOStream::dumpState()
{
    FMESG(
        "TapeDeck { bufferCount = %d/%d, bufferLength = %dB }",
        this->bufferCount,
        TAPEDECK_MAXIMUM_BUFFERS,
        this->bufferLength
    );
}

int FIFOStream::pullRequest()
{
    DMESG("FIFO:pullRequest");

    if( this->bufferCount >= TAPEDECK_MAXIMUM_BUFFERS )
        return DEVICE_NO_RESOURCES;

    ManagedBuffer inBuffer = this->upStream.pull();

    if( this->allowInput && inBuffer.length() > 0 )
    {
        this->buffer[ this->bufferCount++ ] = inBuffer;
        this->bufferLength += inBuffer.length();
    }

    return DEVICE_OK;
}

void FIFOStream::connect( DataSink &sink )
{
    this->downStream = &sink;
    DMESG( "FIFO, Connected" );
}

void FIFOStream::disconnect()
{
    this->downStream = NULL;
    DMESG( "FIFO, Disconnected" );
}

int FIFOStream::getFormat()
{
    DMESG("FIFO:getFormat");
    return this->upStream.getFormat();
}

int FIFOStream::setFormat( int format )
{
    DMESG("FIFO:setFormat");
    return this->upStream.setFormat( format );
}

void FIFOStream::setInputEnable( bool state )
{
    DMESG("FIFO:setInputEnable %d", state );
    this->allowInput = state;
}
void FIFOStream::setOutputEnable( bool state )
{
    DMESG("FIFO:setOutputEnable %d", state );
    this->allowOutput = state;
}