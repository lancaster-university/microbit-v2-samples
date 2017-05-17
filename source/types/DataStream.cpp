#include "DataStream.h"
#include "DeviceComponent.h"
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
    this->pullRequestEventCode = 0;
    this->spaceAvailableEventCode = allocateNotifyEvent();
    this->isBlocking = true;
    this->writers = 0;

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
 * Determines if this stream acts in a synchronous, blocking mode or asynchronous mode. In blocking mode, writes to a full buffer
 * will result in the calling fiber being blocked until space is available. Downstream DataSinks will also attempt to process data
 * immediately as it becomes available. In non-blocking asynchronous mode, writes to a full buffer are dropped and processing of 
 * downstream Datasinks will be deferred.
 */
void DataStream::setBlocking(bool isBlocking)
{
    this->isBlocking = isBlocking;

    // If this is the first time async mode has been used on this stream, allocate the necessary resources.
    if (!isBlocking && this->pullRequestEventCode == 0)
    {
        this->pullRequestEventCode = allocateNotifyEvent();
    
        if(EventModel::defaultEventBus)
            EventModel::defaultEventBus->listen(DEVICE_ID_NOTIFY, pullRequestEventCode, this, &DataStream::onDeferredPullRequest);
    }
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

    DeviceEvent(DEVICE_ID_NOTIFY_ONE, spaceAvailableEventCode);

	return out;
}

/**
 * Issue a pull request to our downstream component, if one has been registered.
 */
void DataStream::onDeferredPullRequest(DeviceEvent)
{
    if (downStream != NULL)
        downStream->pullRequest();
}

/**
 * Determines if a buffer of the given size can be added to the buffer.
 *
 * @param size The number of bytes to add to the buffer.
 * @return true if there is space for "size" bytes in the buffer. false otherwise.
 */
bool DataStream::canPull(int size)
{
    if(bufferCount + writers >= DATASTREAM_MAXIMUM_BUFFERS)
        return false;

    if(preferredBufferSize > 0 && (bufferLength + size > preferredBufferSize))
        return false;

    return true;
}

/**
 * Determines if the DataStream can accept any more data.
 *
 * @return true if there if the buffer is ful, and can accept no more data at this time. False otherwise.
 */
bool DataStream::full()
{
    return !canPull();
}

/**
 * Store the given buffer in our stream, possibly also causing a push operation on our downstream component.
 */
int DataStream::pullRequest()
{
    // If we're defined as non-blocking and no space is available, then there's nothing we can do.
    if (full() && this->isBlocking == false)
        return DEVICE_NO_RESOURCES;

    // As there is either space available in the buffer or we want to block, pull the upstream buffer to release resources there.
    ManagedBuffer buffer = upStream->pull();

    // If the buffer is full or we're behind another fiber, then wait for space to become available.
    if (full() || writers)
        fiber_wake_on_event(DEVICE_ID_NOTIFY, spaceAvailableEventCode);

    if (full() || writers)
    {
        writers++;
        schedule();
        writers--;
    }

	stream[bufferCount] = buffer;
	bufferLength = bufferLength + buffer.length();
	bufferCount++;

	if (downStream != NULL)
    {
        if (this->isBlocking)
            downStream->pullRequest();
        else
            DeviceEvent(DEVICE_ID_NOTIFY, pullRequestEventCode);
    }

	return DEVICE_OK;
}

