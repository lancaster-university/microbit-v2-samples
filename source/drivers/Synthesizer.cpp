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
        launching->generate(-1);
}

/**
  * Class definition for a Synthesizer.
  * A Synthesizer generates a tone waveform based on a number of overlapping waveforms.
  */
Synthesizer::Synthesizer(int sampleRate) : output(*this)
{
	this->bufferSize = 512;
	this->samplePeriodNs = 1000000000 / sampleRate;
	this->setVolume(1023);
    this->active = false;
    this->synchronous = false;
    this->bytesWritten = 0;
    this->status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;
}

/**
 * Implement this function to receive a callback when the device is idling.
 */
void Synthesizer::idleCallback() 
{
    if (bytesWritten && !synchronous && output.canPull(bytesWritten))
    {
        buffer.truncate(bytesWritten);
        output.pullRequest();
        bytesWritten = 0;
    }
}

 
/**
  * Define the central frequency of this synthesizer
  */
int Synthesizer::setFrequency(float frequency)
{
    return setFrequency(frequency, 0);
}

/**
 * Define the central frequency of this synthesizer. 
 * Takes effect immediately, and automatically stops the tone after the given period.
 * @frequency The frequency, in Hz to generate.
 * @period The period, in ms, to play the frequency.
 */
int Synthesizer::setFrequency(float frequency, int period)
{
    // If another fiber is already actively using this resource, we can't service this request.
    if (synchronous)
        return DEVICE_BUSY;

    // record our new intended frequency.
    newPeriodNs = frequency == 0.0 ? 0 : (uint32_t) (1000000000.0 / frequency);

    if (period == 0)
    {
        // We've been asked to play a new tone in the background.
        // If a tone is already playing in the background, we only need to update frequency (already done above). Otherwise also launch a playout fiber.
        if(!active)
        {
            active = true;
            launching = this;
            create_fiber(begin_playback);
        }
    }
    else
    {
        // We've been asked to playout a new note synchronously. Record the period of playback, and start creation of the sample content.
        synchronous = true;
        generate(1000 * period);
        synchronous = false;
    }

    return DEVICE_OK;
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
void Synthesizer::generate(int playoutTimeUs)
{
    int periodNs = newPeriodNs;
    int playoutSoFarNs = 0;
    int position = 0;

    while(playoutTimeUs != 0)
    {
        if (bytesWritten == 0)
            buffer = ManagedBuffer(bufferSize);

        uint16_t *ptr = (uint16_t *) &buffer[bytesWritten];

        while(bytesWritten < bufferSize)
        {
            *ptr = periodNs > 0 ? GENERATE_SAMPLE(amplitude, position, periodNs) : 0;
            bytesWritten += 2;
            ptr++;

            position += samplePeriodNs;
            playoutSoFarNs += samplePeriodNs;

            while (playoutSoFarNs > 1000)
            {
                playoutSoFarNs -= 1000;
                if (playoutTimeUs > 0)
                    playoutTimeUs--;
            }

            while (position >= periodNs)
            {
                position -= periodNs;

                if (periodNs != newPeriodNs)
                {
                    periodNs = newPeriodNs;
                    position = 0;
                }
            }

            if (playoutTimeUs == 0)
                return;
        }

        output.pullRequest();
        bytesWritten = 0;

        // There's now space for another buffer. If we're generating asynchronously and a synchronous request comes in, give control to that fiber.
        if (playoutTimeUs < 0 && synchronous)
        {
            active = false;
            return;
        }
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
