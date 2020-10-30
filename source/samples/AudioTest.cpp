#include "MicroBit.h"
#include "Tests.h"

void
audio_sound_expression_test()
{
    const ManagedString names[] = {
        ManagedString("giggle"),
        ManagedString("happy"),
        ManagedString("hello"),
        ManagedString("mysterious"),
        ManagedString("sad"),
        ManagedString("slide"),
        ManagedString("soaring"),
        ManagedString("spring"),
        ManagedString("twinkle"),
        ManagedString("yawn"),
        ManagedString("")
    };
    
    // uBit.audio.virtualOutputPin.setAnalogPeriodUs(6068);
    for (int i = 0; names[i].length() != 0; ++i) {
        DMESG("sound %s", names[i].toCharArray());
        uBit.audio.setVolume(255);
        uBit.audio.soundExpressions.play(names[i]);
        uBit.sleep(2000);

        uBit.audio.setVolume(85);
        uBit.audio.soundExpressions.play(names[i]);
        uBit.sleep(2000);
    }
    
    while (1) {
        //uBit.display.scroll("...");
        DMESG("PING");
        uBit.sleep(1000);
    }
}

void soundExpressionStop(Event e) {
    uBit.audio.soundExpressions.stop();
}

void soundExpressionStopHello(Event e) {
    // If we're playing a sound with another queued then:
    // stop() will cancel the playing sound
    // the queued sound will start playing
    // playAsync("hello") will wait for the lock
    uBit.audio.soundExpressions.stop();
    uBit.audio.soundExpressions.playAsync("hello");
}

void soundExpressionSoaring(Event e) {
    uBit.audio.soundExpressions.playAsync("soaring");
}

void
audio_sound_expression_stop()
{
    uBit.messageBus.listen(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK, soundExpressionSoaring);
    uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, soundExpressionStop);
    uBit.messageBus.listen(DEVICE_ID_BUTTON_AB, DEVICE_BUTTON_EVT_CLICK, soundExpressionStopHello);
    release_fiber();
}
