/*
The MIT License (MIT)

Copyright (c) 2016 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "DeviceConfig.h"
#include "DeviceTimer.h"
#include "DeviceEvent.h"
#include "CodalCompat.h"
#include "DeviceSystemTimer.h"
#include "LevelDetector.h"
#include "ErrorNo.h"

LevelDetector::LevelDetector(DataSource &source, int highThreshold, int lowThreshold, uint16_t id) : upstream(source)
{
    this->id = id;
    this->level = 0;
    this->sigma = 0;
    this->windowPosition = 0;
    this->windowSize = LEVEL_DETECTOR_DEFAULT_WINDOW_SIZE;
    this->lowThreshold = lowThreshold;
    this->highThreshold = highThreshold;
    this->status |= LEVEL_DETECTOR_INITIALISED;

    // Register with our upstream component
    source.connect(*this);
}

/**
 * Callback provided when data is ready.
 */
int LevelDetector::pullRequest()
{
    ManagedBuffer b = upstream.pull();
    int16_t *data = (int16_t *) &b[0];

    int samples = b.length() / 2;

    for (int i=0; i < samples; i++)
    {
        sigma += abs(*data);
        windowPosition++;

        if (windowPosition == windowSize)
        {
            level = sigma / windowSize;
            sigma = 0;
            windowPosition = 0;

            if ((!(status & LEVEL_DETECTOR_HIGH_THRESHOLD_PASSED)) && level > highThreshold)
            {
                DeviceEvent(id, LEVEL_THRESHOLD_HIGH);
                status |=  LEVEL_DETECTOR_HIGH_THRESHOLD_PASSED;
                status &= ~LEVEL_DETECTOR_LOW_THRESHOLD_PASSED;
            }

            if ((!(status & LEVEL_DETECTOR_LOW_THRESHOLD_PASSED)) && level < lowThreshold)
            {
                DeviceEvent(id, LEVEL_THRESHOLD_LOW);
                status |=  LEVEL_DETECTOR_LOW_THRESHOLD_PASSED;
                status &= ~LEVEL_DETECTOR_HIGH_THRESHOLD_PASSED;
            }
        }

        data++;
    }

    return DEVICE_OK;
}

/*
 * Determines the instantaneous value of the sensor, in SI units, and returns it.
 *
 * @return The current value of the sensor.
 */
int LevelDetector::getValue()
{
    return level;
}


/**
 * Set threshold to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param value the LOW threshold at which a LEVEL_THRESHOLD_LOW will be generated.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int LevelDetector::setLowThreshold(uint16_t value)
{
    // Protect against churn if the same threshold is set repeatedly.
    if (lowThreshold == value)
        return DEVICE_OK;

    // We need to update our threshold 
    lowThreshold = value;

    // Reset any exisiting threshold state, and enable threshold detection.
    status &= ~LEVEL_DETECTOR_LOW_THRESHOLD_PASSED;

    // If a HIGH threshold has been set, ensure it's above the LOW threshold.
    if (highThreshold < lowThreshold)
        setHighThreshold(lowThreshold+1);

    return DEVICE_OK;
}

/**
 * Set threshold to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param value the HIGH threshold at which a LEVEL_THRESHOLD_HIGH will be generated.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int LevelDetector::setHighThreshold(uint16_t value)
{
    // Protect against churn if the same threshold is set repeatedly.
    if (highThreshold == value)
        return DEVICE_OK;

    // We need to update our threshold 
    highThreshold = value;

    // Reset any exisiting threshold state, and enable threshold detection.
    status &= ~LEVEL_DETECTOR_HIGH_THRESHOLD_PASSED;

    // If a HIGH threshold has been set, ensure it's above the LOW threshold.
    if (lowThreshold > highThreshold)
        setLowThreshold(highThreshold - 1);

    return DEVICE_OK;
}

/**
 * Determines the currently defined low threshold.
 *
 * @return The current low threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
 */
int LevelDetector::getLowThreshold()
{
    return lowThreshold;
}

/**
 * Determines the currently defined high threshold.
 *
 * @return The current high threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
 */
int LevelDetector::getHighThreshold()
{
    return highThreshold;
}

/**
 * Set the window size to the given value. The window size defines the number of samples used to determine a sound level.
 * The higher the value, the more accurate the result will be. The lower the value, the more responsive the result will be.
 * Adjust this value to suit the requirements of your applicaiton.
 *
 * @param size The size of the window to use (number of samples).
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int LevelDetector::setWindowSize(int size)
{
    if (size <= 0)
        return DEVICE_INVALID_PARAMETER;

    this->windowSize = size;
    return DEVICE_OK;
}

/**
 * Destructor.
 */
LevelDetector::~LevelDetector()
{
}

