#include "ManagedBuffer.h"
#include "DataStream.h"


#define TAPEDECK_MAXIMUM_BUFFERS 32

namespace codal {

    class TapeDeck : public DataSource, public DataSink
    {
        private:

        ManagedBuffer buffer[TAPEDECK_MAXIMUM_BUFFERS];
        int bufferCount;
        int bufferLength;

        DataSink *downStream;
        DataSource *upStream;

        public:

        TapeDeck( DataSource &upstream );
        ~TapeDeck();

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


    };

}