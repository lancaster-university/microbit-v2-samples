#include "MicroBit.h"
#include "Tests.h"

enum Note {
    C = 262,
    CSharp = 277,
    D = 294,
    Eb = 311,
    E = 330,
    F = 349,
    FSharp = 370,
    G = 392,
    GSharp = 415,
    A = 440,
    Bb = 466,
    B = 494,
    C3 = 131,
    CSharp3 = 139,
    D3 = 147,
    Eb3 = 156,
    E3 = 165,
    F3 = 175,
    FSharp3 = 185,
    G3 = 196,
    GSharp3 = 208,
    A3 = 220,
    Bb3 = 233,
    B3 = 247,
    C4 = 262,
    CSharp4 = 277,
    D4 = 294,
    Eb4 = 311,
    E4 = 330,
    F4 = 349,
    FSharp4 = 370,
    G4 = 392,
    GSharp4 = 415,
    A4 = 440,
    Bb4 = 466,
    B4 = 494,
    C5 = 523,
    CSharp5 = 555,
    D5 = 587,
    Eb5 = 622,
    E5 = 659,
    F5 = 698,
    FSharp5 = 740,
    G5 = 784,
    GSharp5 = 831,
    A5 = 880,
    Bb5 = 932,
    B5 = 988,
};

static Pin *pin = NULL;
static uint8_t pitchVolume = 0xff;

// Pin control as per MakeCode.
static void analogPitch(int frequency, int ms) {
    if (frequency <= 0 || pitchVolume == 0) {
        pin->setAnalogValue(0);
    } else {
        // I don't understand the logic of this value.
        // It is much louder on the real pin.
        int v = 1 << (pitchVolume >> 5);
        // If you flip the order of these they crash on the real pin with E030.
        pin->setAnalogValue(v);
        pin->setAnalogPeriodUs(1000000/frequency);
    }
    if (ms > 0) {
        fiber_sleep(ms);
        pin->setAnalogValue(0);
        fiber_sleep(5);
    }
}

static void playScale() {
    const int beat = 500;
    analogPitch(Note::C5, beat);
    analogPitch(Note::B, beat);
    analogPitch(Note::A, beat);
    analogPitch(Note::G, beat);
    analogPitch(Note::F, beat);
    analogPitch(Note::E, beat);
    analogPitch(Note::D, beat);
    analogPitch(Note::C, beat);
}

void audio_virtual_pin_melody()
{
    pin = &uBit.audio.virtualOutputPin;
    playScale();

    // For comparison:
    pin = &uBit.io.P0;
    playScale();
}

void audio_sound_expression_test()
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
        // "sad" but with an additional zero-duration effect which previously caused errors:
        ManagedString("010232279000001440226608881023012800000000240000000000000000000000000000,000000440000000440044008880000012800000000240000000000000000000000000000,310232226070801440162408881023012800000100240000000000000000000000000000,310231623093602440093908880000012800000100240000000000000000000000000000"),
        // Just a zero-duration frame.
        ManagedString("000000440000000440044008880000012800000000240000000000000000000000000000"),
        ManagedString("")
    };
    
    // uBit.audio.virtualOutputPin.setAnalogPeriodUs(6068);
    while (1) {
        for (int i = 0; names[i].length() != 0; ++i) {
            DMESG("sound %s", names[i].toCharArray());
            uBit.audio.setVolume(255);
            uBit.audio.soundExpressions.play(names[i]);

            uBit.audio.setVolume(85);
            uBit.audio.soundExpressions.play(names[i]);
        }
    }
}