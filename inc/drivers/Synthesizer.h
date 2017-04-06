#ifndef CODAL_SYNTHESIZER_H
#define CODAL_SYNTHESIZER_H

#include "DataStream.h"

#define SYNTHESIZER_SAMPLE_RATE		44100

/**
  * Class definition for DataStream.
  * A Datastream holds a number of ManagedBuffer references, provides basic flow control through a push/pull mechanism
  * and byte level access to the datastream, even if it spans different buffers.
  */
class Synthesizer : public DataSource
{
	ManagedBuffer buffer;

	uint32_t position;
	uint32_t sampleRate;
	uint32_t periodNs;
	uint32_t samplePeriodNs;
	uint32_t newPeriodNs;
	uint32_t amplitude;
	uint32_t bufferSize;

    public:

	DataStream output;

    /**
      * Default Constructor. 
      * Creates an empty DataStream.
      *
      * @param upstream the component that will normally feed this datastream with data.
      */
    Synthesizer(int sampleRate = SYNTHESIZER_SAMPLE_RATE);

    /**
      * Destructor. 
      * Removes all resources held by the instance.
      */
    ~Synthesizer();

	/**
	* Define the central frequency of this synthesizer. Takes effect at the start of the next waveform period.
	* @frequency Te frequency, in Hz to generate.
	*/
	void setFrequency(float frequency);

	/**
	* Define the volume of the wave to generate.
	* @param volume The new output volume, in the range 0..1023
	* @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
	*/
	int setVolume(int volume);

	/**
	* Define the size of the audio buffer to hold. The larger the buffer, the lower the CPU overhead, but the longer the delay.
	* @param size The new bufer size to use.
	* @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
	*/
	int setBufferSize(int size);

	/**
	 * Provide the next available ManagedBuffer to our downstream caller, if available.
	 */
	virtual ManagedBuffer pull();

	/**
	* Creates the next audio buffer, and attmepts to queue this on the output stream.
	*/
	void generate();

};
#endif

