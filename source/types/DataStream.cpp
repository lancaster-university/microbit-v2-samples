#include "DataStream.h"
#include "DeviceComponent.h"
#include "DeviceMessageBus.h"
#include "DeviceFiber.h"
#include "ErrorNo.h"

/**
* Default implementation of DataSource and DataSink classes.
*/
ManagedBuffer DataSource::pull() 
{ 
	return ManagedBuffer();
}

void DataSource::connect(DataSink& )
{
}

int DataSink::pullRequest() 
{
	return DEVICE_NOT_SUPPORTED;
}

/**
  * Class definition for DataStream.
  * A Datastream holds a number of ManagedBuffer references, provides basic flow control through a push/pull mechanism
  * and byte level access to the datastream, even if it spans different buffers.
  */
DataStream::DataStream(DataSource &upstream)
{
    this->bufferCount = 0;
    this->bufferLength = 0;
    this->preferredBufferSize = 0;
    this->notifyEventCode = allocateNotifyEvent();

    this->downStream = NULL;
    this->upStream = &upstream;
}

/**
 * Destructor. 
 * Removes all resources held by the instance.
 */
DataStream::~DataStream()
{
}

/**
 * Determines the value of the given byte in the buffer.
 *
 * @param position The index of the byte to read.
 * @return The value of the byte at the given position, or DEVICE_INVALID_PARAMETER.
 */
int DataStream::get(int position)
{
	for (int i = 0; i < bufferCount; i++)
	{
		if (position < stream[i].length())
			return stream[i].getByte(position);

		position = position - stream[i].length();
	}

	return DEVICE_INVALID_PARAMETER;
}

/**
 * Sets the byte at the given index to value provided.
 * @param position The index of the byte to change.
 * @param value The new value of the byte (0-255).
 * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
 *
 */
int DataStream::set(int position, uint8_t value)
{
	for (int i = 0; i < bufferCount; i++)
	{
		if (position < stream[i].length())
		{
			stream[i].setByte(position, value);
			return DEVICE_OK;
		}

		position = position - stream[i].length();
	}

	return DEVICE_INVALID_PARAMETER;
}

/**
 * Gets number of bytes that are ready to be consumed in this data stream. 
 * @return The size in bytes.
 */
int DataStream::length()
{
	return this->bufferLength;
}

/**
 * Determines if any of the data currently flowing through this stream is held in non-volatile (FLASH) memory.
 * @return true if one or more of the ManagedBuffers in this stream reside in FLASH memory, false otherwise.
 */
bool DataStream::isReadOnly()  
{ 
    bool r = true;

    for (int i=0; i<bufferCount;i++)
        if (stream[i].isReadOnly() == false)
            r = false;

    return r;
}

/**
 * Define a downstream component for data stream.
 *
 * @sink The component that data will be delivered to, when it is availiable
 */
void DataStream::connect(DataSink &sink)
{
	this->downStream = &sink;
}

/**
 * Define a downstream component for data stream.
 *
 * @sink The component that data will be delivered to, when it is availiable
 */
void DataStream::disconnect()
{
	this->downStream = NULL;
}

/**
 * Determine the number of bytes that are currnetly buffered before blocking subsequent push() operations.
 * @return the current preferred buffer size for this DataStream
 */
int DataStream::getPreferredBufferSize()
{
	return preferredBufferSize;
}

/**
 * Define the number of bytes that should be buffered before blocking subsequent push() operations.
 * @param size The number of bytes to buffer.
 */
void DataStream::setPreferredBufferSize(int size)
{
	this->preferredBufferSize = size;
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer DataStream::pull()
{
	ManagedBuffer out = stream[0];

	//
	// A simplistic FIFO for now. Copy cost is actually pretty low because ManagedBuffer is a managed type,
	// so we're just moving a few references here.
	//
	if (bufferCount > 0)
	{
		for (int i = 0; i < bufferCount-1; i++)
			stream[i] = stream[i + 1];

        stream[bufferCount-1] = ManagedBuffer();

		bufferCount--;
		bufferLength = bufferLength - out.length();
	}

    DeviceEvent(DEVICE_ID_NOTIFY, notifyEventCode);

	return out;
}

/**
 * Store the given buffer in our stream, possibly also causing a push operation on our downstream component.
 */
int DataStream::pullRequest()
{
	if (bufferCount == DATASTREAM_MAXIMUM_BUFFERS || bufferLength > preferredBufferSize)
        fiber_wait_for_event(DEVICE_ID_NOTIFY, notifyEventCode);

	stream[bufferCount] = upStream->pull();
	bufferLength = bufferLength + stream[bufferCount].length();
	bufferCount++;
	
	if (downStream != NULL)
		downStream->pullRequest();

	return DEVICE_OK;
}

