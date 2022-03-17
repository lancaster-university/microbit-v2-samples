#include "FIFOStream.h"
#include "ErrorNo.h"
#include "DataStream.h"
#include "ManagedBuffer.h"
#include "CodalDmesg.h"
#include "MessageBus.h"

using namespace codal;

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
    return (this->bufferLength > 0) && this->allowOutput;
}

ManagedBuffer FIFOStream::pull()
{
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

    return ManagedBuffer();
}

int FIFOStream::length()
{
    return this->bufferLength;
}

bool FIFOStream::isFull() {
    return this->bufferCount < TAPEDECK_MAXIMUM_BUFFERS;
}

void FIFOStream::dumpState()
{
    DMESG(
        "TapeDeck { bufferCount = %d/%d, bufferLength = %dB }",
        this->bufferCount,
        TAPEDECK_MAXIMUM_BUFFERS,
        this->bufferLength
    );
}

int FIFOStream::pullRequest()
{
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
}

void FIFOStream::disconnect()
{
    this->downStream = NULL;
}

int FIFOStream::getFormat()
{
    return this->upStream.getFormat();
}

int FIFOStream::setFormat( int format )
{
    return this->upStream.setFormat( format );
}

void FIFOStream::setInputEnable( bool state )
{
    this->allowInput = state;
}
void FIFOStream::setOutputEnable( bool state )
{
    this->allowOutput = state;
}