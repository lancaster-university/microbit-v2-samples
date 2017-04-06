#ifndef CODAL_DATA_STREAM_H
#define CODAL_DATA_STREAM_H

#include "ManagedBuffer.h"

#define DATASTREAM_MAXIMUM_BUFFERS      4

/**
 * Interface definition for a DataSource.
 */
class DataSink
{
	public:

	virtual int pullRequest(); 
};

/**
* Interface definition for a DataSource.
*/
class DataSource
{
	public:

	virtual ManagedBuffer pull();
	virtual void connect(DataSink &sink);
};

/**
  * Class definition for DataStream.
  * A Datastream holds a number of ManagedBuffer references, provides basic flow control through a push/pull mechanism
  * and byte level access to the datastream, even if it spans different buffers.
  */
class DataStream : public DataSource, public DataSink
{
    ManagedBuffer stream[DATASTREAM_MAXIMUM_BUFFERS];
    int bufferCount;
    int bufferLength;
    int preferredBufferSize;
	bool blocked;

    DataSink *downStream;
    DataSource *upStream;

    public:

    /**
      * Default Constructor. 
      * Creates an empty DataStream.
      *
      * @param upstream the component that will normally feed this datastream with data.
      */
    DataStream(DataSource &upstream);

    /**
      * Destructor. 
      * Removes all resources held by the instance.
      */
    ~DataStream();

    /**
      * Determines the value of the given byte in the buffer.
      *
      * @param position The index of the byte to read.
      * @return The value of the byte at the given position, or DEVICE_INVALID_PARAMETER.
      */
    int get(int position);

    /**
      * Sets the byte at the given index to value provided.
      * @param position The index of the byte to change.
      * @param value The new value of the byte (0-255).
      * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
      *
      */
    int set(int position, uint8_t value);

    /**
      * Gets number of bytes that are ready to be consumed in this data stream. 
      * @return The size in bytes.
      */
    int length();

    /**
     * Determines if any of the data currently flowing through this stream is held in non-volatile (FLASH) memory.
     * @return true if one or more of the ManagedBuffers in this stream reside in FLASH memory, false otherwise.
     */
	bool isReadOnly();

    /**
     * Define a downstream component for data stream.
     *
     * @sink The component that data will be delivered to, when it is availiable
     */
    virtual void connect(DataSink &sink);

    /**
     * Define a downstream component for data stream.
     *
     * @sink The component that data will be delivered to, when it is availiable
     */
    void disconnect();

    /**
     * Determine the number of bytes that are currnetly buffered before blocking subsequent push() operations.
     * @return the current preferred buffer size for this DataStream
     */
    int getPreferredBufferSize();

    /**
     * Define the number of bytes that should be buffered before blocking subsequent push() operations.
     * @param size The number of bytes to buffer.
     */
    void setPreferredBufferSize(int size);

	/**
	 * Provide the next available ManagedBuffer to our downstream caller, if available.
	 */
	virtual ManagedBuffer pull();

	/**
	 * Deliver the next available ManagedBuffer to our downstream caller.
	 */
	virtual int pullRequest();

};
#endif

