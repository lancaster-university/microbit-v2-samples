#include "ManagedBuffer.h"
#include "DataStream.h"


#define TAPEDECK_MAXIMUM_BUFFERS 256

namespace codal {

    class FIFOStream : public DataSource, public DataSink
    {
        private:

        ManagedBuffer buffer[TAPEDECK_MAXIMUM_BUFFERS];
        int bufferCount;
        int bufferLength;
        int notifyID;

        bool allowInput;
        bool allowOutput;

        DataSink *downStream;
        DataSource &upStream;

        public:

        FIFOStream( DataSource &source );
        ~FIFOStream();

        virtual ManagedBuffer pull();
        virtual int pullRequest();
    	virtual void connect( DataSink &sink );
        virtual void disconnect();
        virtual int getFormat();
        virtual int setFormat( int format );
        int length();
        void dumpState();

        bool canPull();
        bool isFull();

        void setInputEnable( bool state );
        void setOutputEnable( bool state );


    };

}