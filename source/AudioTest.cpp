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
