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
#include "SystemClock.h"
#include "DataStream.h"

#ifndef LEVEL_DETECTOR_H
#define LEVEL_DETECTOR_H


/**
  * Sensor events
  */
#define LEVEL_THRESHOLD_LOW                           1
#define LEVEL_THRESHOLD_HIGH                          2

/**
 * Status values
 */
#define LEVEL_DETECTOR_INITIALISED                       0x01
#define LEVEL_DETECTOR_HIGH_THRESHOLD_PASSED             0x02
#define LEVEL_DETECTOR_LOW_THRESHOLD_PASSED              0x04

/**
 * Default configuration values
 */
#define LEVEL_DETECTOR_DEFAULT_WINDOW_SIZE              128 

class LevelDetector : public DeviceComponent, public DataSink
{
public:
   
    // The stream component that is serving our data 
    DataSource      &upstream;          // The component producing data to process
    int             highThreshold;      // threshold at which a HIGH event is generated
    int             lowThreshold;       // threshold at which a LOW event is generated
    int             windowSize;         // The number of samples the make up a level detection window.
    int             windowPosition;     // The number of samples used so far in the calculation of a window.
    int             level;              // The current, instantaneous level.
    int             sigma;              // Running total of the samples in the current window.


    /**
      * Creates a component capable of measuring and thresholding stream data
      * 
      * @param source a DataSource to measure the level of.
      * @param highThreshold the HIGH threshold at which a LEVEL_THRESHOLD_HIGH event will be generated 
      * @param lowThreshold the HIGH threshold at which a LEVEL_THRESHOLD_LOW event will be generated 
      * @param id The id to use for the message bus when transmitting events.
      */
    LevelDetector(DataSource &source, int highThreshold, int lowThreshold, uint16_t id = DEVICE_ID_SYSTEM_LEVEL_DETECTOR);

    /**
     * Callback provided when data is ready.
     */
	virtual int pullRequest();

    /*
     * Determines the instantaneous value of the sensor, in SI units, and returns it.
     *
     * @return The current value of the sensor.
     */
    int getValue();

    /**
     * Set threshold to the given value. Events will be generated when these thresholds are crossed.
     *
     * @param value the LOW threshold at which a ANALOG_THRESHOLD_LOW will be generated.
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
     */
    int setLowThreshold(uint16_t value);

    /**
     * Set threshold to the given value. Events will be generated when these thresholds are crossed.
     *
     * @param value the HIGH threshold at which a ANALOG_THRESHOLD_HIGH will be generated.
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
     */
    int setHighThreshold(uint16_t value);

    /**
     * Determines the currently defined low threshold.
     *
     * @return The current low threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
     */
    int getLowThreshold();

    /**
     * Determines the currently defined high threshold.
     *
     * @return The current high threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
     */
    int getHighThreshold();

    /**
     * Set the window size to the given value. The window size defines the number of samples used to determine a sound level.
     * The higher the value, the more accurate the result will be. The lower the value, the more responsive the result will be.
     * Adjust this value to suit the requirements of your applicaiton.
     *
     * @param size The size of the window to use (number of samples).
     *
     * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
     */
    int setWindowSize(int size);

    /**
     * Destructor.
     */
    ~LevelDetector();

};

#endif
