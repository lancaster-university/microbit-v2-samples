#include "MicroBit.h"
#include "CodalAssert.h"
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"
#include "StreamRecording.h"
#include "Tests.h"

/**
 * Note - These tests use CodalAssert.h - so require the following codal.json options
 * be enabled:
 * CODAL_ENABLE_ASSERT: 1
 * 
 * To cause the first assertion failure to halt the processor, also include:
 * CODAL_ASSERT_PANIC: 1
 */

extern MicroBit uBit;

/**
 * Tests if the mic auto activation (and ADC activation) works correctly
 */
void stream_test_mic_activate() {
    assert( uBit.audio.isMicrophoneEnabled() == false, "Microphone was enabled on startup?" );
    assert( uBit.audio.mic->output.isFlowing() == false, "isFlowing() should be false on startup." );
    int level = uBit.audio.levelSPL->getValue();
    assert( uBit.audio.isMicrophoneEnabled(), "getValue() failed to turn the MIC on!" );
    assert( uBit.audio.mic->output.isFlowing(), "isFlowing() was not true after mic activation" );
    assert( level > 0, "Detected level appears to be zero? Defective hardware?" );
    assert_pass( NULL );
}

void stream_test_getValue_interval() {
    uBit.audio.levelSPL->getValue(); // Wake the stream :)
    for( int i=0; i<10; i++ ) {
        assert( uBit.audio.isMicrophoneEnabled(), "Microphone should still be active" );
        assert( uBit.audio.mic->output.isFlowing(), "isFlowing() should still be true" );
        int level = uBit.audio.levelSPL->getValue();
        assert( level > 0, "Detected level appears to be zero? Defective hardware?" );
        uBit.sleep( CODAL_STREAM_IDLE_TIMEOUT_MS / 2 );
    }
    uBit.sleep( CODAL_STREAM_IDLE_TIMEOUT_MS * 2 ); // Ensure we go quiscient
    assert( uBit.audio.isMicrophoneEnabled() == false, "Microphone should be shut down after 2 * CODAL_STREAM_IDLE_TIMEOUT_MS" );
    assert( uBit.audio.mic->output.isFlowing() == false, "isFlowing() should be false after 2 * CODAL_STREAM_IDLE_TIMEOUT_MS" );
    assert_pass( NULL );
}

void stream_test_record() {
    uBit.audio.requestActivation();
    static SplitterChannel * input = uBit.audio.splitter->createChannel();
    static StreamRecording * recording = new StreamRecording( *input );
    static MixerChannel * output = uBit.audio.mixer.addChannel( *recording );

    input->requestSampleRate( 11000 );
    output->setSampleRate( 11000 );
    output->setVolume( CONFIG_MIXER_INTERNAL_RANGE * 0.8 ); // 80% volume

    uBit.display.printChar( '3', 1000 );
    uBit.display.printChar( '2', 1000 );
    uBit.display.printChar( '1', 1000 );

    uBit.display.printChar( 'R' );
    input->requestSampleRate( 11000 );
    recording->recordAsync();
    while( recording->isRecording() ) {
        uBit.display.printChar( '~' );
        uBit.sleep( 100 );
        uBit.display.printChar( '-' );
        uBit.sleep( 100 );
    }
    uBit.display.printChar( 'X' );

    uBit.sleep( 1000 );

    uBit.display.printChar( 'P' );
    output->setSampleRate( 18000 );
    recording->playAsync();
    while( recording->isPlaying() ) {
        uBit.display.printChar( '>' );
        uBit.sleep( 100 );
        uBit.display.printChar( ' ' );
        uBit.sleep( 100 );
    }
    uBit.display.printChar( 'X' );

    uBit.sleep( 1000 );

    recording->erase();
}

static const int STRSR_SAMPLE_RATE = 11000;

static void strsr_handle_buttonA(MicroBitEvent) {
    static SplitterChannel *splitterChannel = uBit.audio.splitter->createChannel();
    splitterChannel->requestSampleRate(STRSR_SAMPLE_RATE);
    static StreamRecording *recording = new StreamRecording(*splitterChannel);
    static MixerChannel *channel = uBit.audio.mixer.addChannel(*recording, STRSR_SAMPLE_RATE);

    DMESG( "Actual sample rate: %d (requested %d)", (int)splitterChannel->getSampleRate(), STRSR_SAMPLE_RATE );

    MicroBitAudio::requestActivation();
    channel->setVolume(75.0);
    uBit.audio.mixer.setVolume(512);
    uBit.audio.mixer.setSilenceLevel( 0 );
    uBit.audio.setPinEnabled( true );

    uBit.display.clear();
    uBit.audio.levelSPL->setUnit(LEVEL_DETECTOR_SPL_8BIT);

    splitterChannel->requestSampleRate( STRSR_SAMPLE_RATE );

    DMESG( "RECORDING" );
    recording->recordAsync();
    bool showR = true;
    while (uBit.buttonA.isPressed()) {
        if( uBit.logo.isPressed() ) {
            splitterChannel->requestSampleRate( abs((uBit.accelerometer.getRoll()-90) * 100) );
            DMESG( "Sample Rate: %d (mic = %d)", (int)splitterChannel->getSampleRate(), (int)uBit.audio.mic->getSampleRate() );
        } else {
            if( uBit.buttonB.isPressed() )
                splitterChannel->requestSampleRate( 5000 );
            else
                splitterChannel->requestSampleRate( STRSR_SAMPLE_RATE );
        }
        
        if (showR)
            uBit.display.print("R");
        else
            uBit.display.clear();
        uBit.sleep(150);
    }
    recording->stop();
    DMESG( "STOPPED" );

    DMESG( "PLAYING" );
    recording->playAsync();
    while (recording->isPlaying()) {
        if (showR)
            uBit.display.print("P");
        else
            uBit.display.clear();
        uBit.sleep(20);
    }
    recording->erase();
    uBit.display.clear();
    DMESG( "STOPPED" );
}

void stream_test_recording_sample_rates() {
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_DOWN, strsr_handle_buttonA);

    while (true) {
        uBit.sleep(1000);
    }
}

void stream_test_all() {
    stream_test_mic_activate();
    stream_test_getValue_interval();
    assert_pass( NULL );
}