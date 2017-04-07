#include "Synthesizer.h"
#include "DeviceFiber.h"
#include "ErrorNo.h"

/*
 * Simple internal helper funtion that creates a fiber within the givien Synthesizer to handle playback
 */
static Synthesizer *launching = NULL;
void begin_playback()
{
    if (launching)
        launching->generate();
}

/**
  * Class definition for a Synthesizer.
  * A Synthesizer generates a tone waveform based on a number of overlapping waveforms.
  */
Synthesizer::Synthesizer(int sampleRate) : output(*this)
{
	this->sampleRate = sampleRate;
	this->bufferSize = 512;

	this->position = 0;
	this->samplePeriodNs = 1000000000 / sampleRate;
	this->setFrequency(261.62);
	this->setVolume(1023);
	this->periodNs = newPeriodNs;

    launching = this;
    create_fiber(begin_playback);
}

/**
  * Define the central frequency of this synthesizer
  */
void Synthesizer::setFrequency(float frequency)
{
	this->newPeriodNs = (uint32_t) (1000000000.0 / frequency);
}

/**
 * Destructor. 
 * Removes all resources held by the instance.
 */
Synthesizer::~Synthesizer()
{
}

/**
 * Creates the next audio buffer, and attmepts to queue this on the output stream.
 */
void Synthesizer::generate()
{
    while(1)
    {
        buffer = ManagedBuffer(bufferSize);

        uint16_t *ptr = (uint16_t *) &buffer[0];

        for (int i = 0; i < buffer.length() / 2; i++)
        {
            *ptr = periodNs > 0 ? (amplitude * position) / periodNs : 0;
            position += samplePeriodNs;

            if (position > periodNs)
            {
                position -= periodNs;
                periodNs = newPeriodNs;
            }

            ptr++;
        }

        output.pullRequest();
    }
}

/**
* Define the volume of the wave to generate.
* @param volume The new output volume, in the range 0..1023
*/
int Synthesizer::setVolume(int volume)
{
	if (volume < 0 || volume > 1023)
		return DEVICE_INVALID_PARAMETER;

	amplitude = volume;

	return DEVICE_OK;
}

/**
* Define the size of the audio buffer to hold. The larger the buffer, the lower the CPU overhead, but the longer the delay.
* @param size The new bufer size to use, in bytes.
* @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
*/
int Synthesizer::setBufferSize(int size)
{
	if (bufferSize <= 0)
		return DEVICE_INVALID_PARAMETER;

	this->bufferSize = size;
	return DEVICE_OK;
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer Synthesizer::pull()
{
	return buffer;
}
