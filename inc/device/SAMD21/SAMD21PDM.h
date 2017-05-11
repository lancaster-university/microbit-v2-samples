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
#include "DevicePin.h"
#include "SAMD21DMAC.h"
#include "DataStream.h"

#ifndef SAMD21PDM_H
#define SAMD21PDM_H

//
// RAW buffer size for PDM data from a MEMS microphone, in bytes.
// n.b. this is required to be word aligned (multiple of 4 bytes), 
// but TWO buffers of this size are created in a double buffer configuration.
//
#ifndef SAMD21_PDM_BUFFER_SIZE
#define SAMD21_PDM_BUFFER_SIZE         256 
#endif

// The number of buffers to cycle through before reporting data back to high layers 
// (used to avoid providing unbalanced samples at the start of use).
#define SAMD21_START_UP_DELAY          3 

//
// Event codes
//
#define SAMD21_PDM_DATA_READY           1

class SAMD21PDM : public DeviceComponent, public DmaComponent, public DataSource
{

private:
    bool            enabled;                                // Determines if this component is actively receiving data.
    int             invalid;                                // Detemrines if this component has received sufficient data to provide valid output.
	ManagedBuffer   buffer;                                 // A reference counted stream buffer used to hold PCM sample data.
    uint32_t        outputBufferSize;                       // The size of our output buffer.
	uint32_t        sampleRate;                             // The PCM output target sample rate (in bps).
    int16_t         *out;                                   // Write pointer into the output PCM buffer;

    uint8_t         rawPDM1[SAMD21_PDM_BUFFER_SIZE];        // A statically alloctaed buffer into which PDM data is transferred via DMA.
    uint8_t         rawPDM2[SAMD21_PDM_BUFFER_SIZE];        // A statically alloctaed buffer into which PDM data is transferred via DMA.
    uint8_t *       pdmDataBuffer;                          // pointer to the PDM buffer that's ready for processing
    uint8_t *       pdmReceiveBuffer;                       // pointer ot the PDM buffer that's ready for receving data from the PDM hardware.

	uint32_t        clockRate;                              // The bit rate at which PDM data is received (in bps).
    uint32_t        overSamplingRate;                       // The number of PDM samples needed to generate one PCM sample (rounded down).
    uint32_t        overSamplingErrorPeriod;                // The remainder of the PDM samples used to generate PCM samples per buffer.
    uint32_t        pdmCount;                               // The number of pdmSampled used so far in the generation of a PCM sample.

    int             s2_sum1;                                // CIC filter parameters.
    int             s2_sum2;
    int             s2_comb1_1;
    int             s2_comb1_2;
    int             s2_comb2_1;
    int             s2_comb2_2;
    int             avg;
    int             sum;
    int             samples;

    int             dmaChannel;                             // The DMA channel used by this component
    SAMD21DMAC      &dmac;                                  // The DMA controller used by this component

public:
   
	DataStream output;

    /**
      * Constructor for an instance of a PDM input (typically microphone),
      *
      * @param sd The pin the PDM data input is connected to.
      * @param sck The pin the PDM clock is conected to.
      * @param dma The DMA controller to use for data transfer.
      * @param sampleRate the rate at which samples are generated in the output buffer (in Hz)
      * @param clockRate the rate at which this PDM source generates samples. (in Hz, valid values int he range 1MHz...8MHz)
      * @param id The id to use for the message bus when transmitting events.
      */
    SAMD21PDM(DevicePin &sd, DevicePin &sck, SAMD21DMAC &dma, int sampleRate, int clockRate=1000000, uint16_t id = DEVICE_ID_SYSTEM_MICROPHONE);

	/**
	 * Provide the next available ManagedBuffer to our downstream caller, if available.
	 */
	virtual ManagedBuffer pull();

	/**
     * Update our reference to a downstream component.
	 */
	virtual void connect(DataSink &sink);

    /**
     * Interrupt callback when playback of DMA buffer has completed
     */
    virtual void dmaTransferComplete();

    /**
     * Enable this component
     */
    void enable();

    /**
     * Disable this component
     */
    void disable();

private:

    void startDMA();
    void decimate(DeviceEvent);
};

#endif
