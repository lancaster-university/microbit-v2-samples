#include "MicroBit.h"

MicroBit uBit;

class RawMicAdcSink : public CodalComponent, public DataSink {
  public:
    DataSource      &upstream;
    int             NrOfPullRequests = 0;

    RawMicAdcSink(DataSource &source): upstream(source) {
        upstream.connect(*this);
    }

    virtual int pullRequest() {
        ManagedBuffer b = upstream.pull();

        uint8_t *data = &b[0];
        int SampleArrRaw[128];
        int format = upstream.getFormat();
        int skip = 2;
        int windowSize = 128;

        if (format != DATASTREAM_FORMAT_16BIT_SIGNED) {
            uBit.serial.printf("Upstream data format enexpected: %d", format);
            uBit.sleep(100);
            uBit.serial.send("\r\n", SYNC_SPINWAIT);
            uBit.panic(123);
        }

        int samples = b.length() / skip;

        while(samples){
            // ensure we use at least windowSize number of samples (128)
            if (samples < windowSize) {
                break;
            }

            uint8_t *ptr = data;
            uint8_t *end = data + windowSize;

            bool dumpToSerial = false;
            uint8_t SampCntr = 0;

            while (ptr < end) {
                int TempSample = StreamNormalizer::readSample[format](ptr);
                SampleArrRaw[SampCntr] = TempSample;
                SampCntr++;
                ptr += skip;

                if (TempSample == -30584) {
                    dumpToSerial = true;
                }
            }
            if (dumpToSerial) {
                uBit.serial.printf("pullRequest() number %d:\r\n", NrOfPullRequests);
                for (int i = 0; i < SampCntr; i++)
                    uBit.serial.printf("%d\t", SampleArrRaw[i]);
                uBit.serial.send("\r\n\r\n");
            }

            samples -= windowSize;
            data += windowSize;
        }

        NrOfPullRequests++;

        return DEVICE_OK;
    }
};


int main() {
    uBit.init();

    uBit.serial.send("Start:\r\n");

    RawMicAdcSink* micAdcSink = new RawMicAdcSink(*(uBit.audio.rawSplitter->createChannel()));

    uBit.io.P2.getAnalogValue();
    uBit.audio.virtualOutputPin.setAnalogPeriodUs(1000000 / 2600);
    uBit.audio.setSpeakerEnabled(false);    // This line is just to avoid the annoying tone being played

    while (true) {
        uBit.sleep(1);
    }
}
