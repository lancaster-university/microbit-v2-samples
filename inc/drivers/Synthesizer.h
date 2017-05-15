#ifndef CODAL_SYNTHESIZER_H
#define CODAL_SYNTHESIZER_H

#include "DataStream.h"

#define SYNTHESIZER_SAMPLE_RATE		10000

#define SAWTOOTH_GENERATOR(amplitude, position, period) ((amplitude * position) / period)
#define SQUAREWAVE_GENERATOR(amplitude, position, period) (position < (period >> 2) ? amplitude : 0)

#define GENERATE_SAMPLE SQUAREWAVE_GENERATOR

/**
  * Class definition for DataStream.
  * A Datastream holds a number of ManagedBuffer references, provides basic flow control through a push/pull mechanism
  * and byte level access to the datastream, even if it spans different buffers.
  */
class Synthesizer : public DataSource, public DeviceComponent
{
	int     samplePeriodNs;        // The length of a single sample, in nanoseconds.
	int     bufferSize;            // The number of samples to create in a single buffer before scheduling it for playback

    int     newPeriodNs;           // new period of waveform, if change has been requested.
	int     amplitude;             // The maximum amplitude of the wave to generate (the volume of the output)
    bool    active;                // Determines if background playback of audio is currently active.
    bool    synchronous;           // Determines if a synchronous mode of operation has been requested. 

    ManagedBuffer buffer;          // Playout buffer.
    int     bytesWritten;          // Number of bytes written to the output buffer.

    public:

	DataStream output;

    /**
      * Default Constructor. 
      * Creates an empty DataStream.
      *
      * @param sampleRate The sample rate at which this synthesizer will produce data.
      */
    Synthesizer(int sampleRate = SYNTHESIZER_SAMPLE_RATE);

    /**
      * Destructor. 
      * Removes all resources held by the instance.
      */
    ~Synthesizer();

	/**
	* Define the central frequency of this synthesizer. Takes effect at the start of the next waveform period.
	* @frequency The frequency, in Hz to generate.
	*/
	int setFrequency(float frequency);

	/**
	* Define the central frequency of this synthesizer. Takes effect at the start of the next waveform period.
	* @frequency The frequency, in Hz to generate.
    * @period The period, in ms, to play the frequency.
	*/
	int setFrequency(float frequency, int period);

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
     * Implement this function to receive a callback when the device is idling.
     */
    virtual void idleCallback();

	/**
	* Creates the next audio buffer, and attmepts to queue this on the output stream.
	*/
	void generate(int playoutTimeUs);

};
#endif

