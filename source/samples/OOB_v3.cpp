#include "MicroBit.h"
#include "Synthesizer.h"
#include "StreamRecording.h"
#include "LowPassFilter.h"
#include "Tests.h"

const char * const heart =
    "000,255,000,255,000\n"
    "255,255,255,255,255\n"
    "255,255,255,255,255\n"
    "000,255,255,255,000\n"
    "000,000,255,000,000\n";
static const MicroBitImage HEART(heart);

const char * const happy =
    "000,000,000,000,000\n"
    "000,255,000,255,000\n"
    "000,000,000,000,000\n"
    "255,000,000,000,255\n"
    "000,255,255,255,000\n";
static const MicroBitImage HAPPY(happy);

const char * const sad =
    "000,000,000,000,000\n"
    "000,255,000,255,000\n"
    "000,000,000,000,000\n"
    "000,255,255,255,000\n"
    "255,000,000,000,255\n";
static const MicroBitImage SAD(sad);

const char * const silent =
    "000,255,000,255,000\n"
    "000,000,000,000,000\n"
    "000,000,000,000,000\n"
    "255,255,255,255,255\n"
    "000,000,000,000,000\n";
static const MicroBitImage SILENT(silent);

const char * const singing =
    "000,255,000,255,000\n"
    "000,000,000,000,000\n"
    "000,255,255,255,000\n"
    "255,000,000,000,255\n"
    "000,255,255,255,000\n";
static const MicroBitImage SINGING(singing);

const char * const asleep =
    "000,000,000,000,000\n"
    "255,255,000,255,255\n"
    "000,000,000,000,000\n"
    "000,255,255,255,000\n"
    "000,000,000,000,000\n";
static const MicroBitImage ASLEEP(asleep);

const char * const sun =
    "255,000,255,000,255\n"
    "000,255,255,255,000\n"
    "255,255,255,255,255\n"
    "000,255,255,255,000\n"
    "255,000,255,000,255\n";
static const MicroBitImage SUN(sun);

const char * const moon =
    "000,000,255,255,000\n"
    "000,000,000,255,255\n"
    "000,000,000,255,255\n"
    "000,000,000,255,255\n"
    "000,000,255,255,000\n";
static const MicroBitImage MOON(moon);

// MakeCode melodies in the format NOTE[octave][:duration]
static const int DEFAULT_TEMPO_BPM = 120;
static const int MS_PER_BPM = (60000 / DEFAULT_TEMPO_BPM) / 4;
static const int NOTE_LEN = 6;
#define TUNE_LEN(tune) (sizeof(tune) / sizeof(tune[0]))

static const char MELODY_POWER_UP[][NOTE_LEN] = {"G4:1", "C5", "E", "G:2", "E:1", "G:3"};
static const char MELODY_POWER_DOWN[][NOTE_LEN] = {"G5:1", "D#", "C", "G4:2", "B5:1", "C:3"};


/**
 * Displays a vertical bar graph based on the `value` and `high` arguments.
 *
 * Function adapted from the MakeCode project, Copyright Microsoft Corporation
 * MIT Licensed (MIT):
 * https://github.com/microsoft/pxt-microbit/blob/v5.1.25/libs/core/led.ts#L42-L83
 * https://github.com/microsoft/pxt-microbit/blob/v5.1.25/LICENSE.txt
 *
 * @param value - Current value to plot.
 * @param high - Maximum value.
 */
static void plotBarGraph(uint32_t value, int high) {
    float v = (float)value / (float)high;
    float dv = 1.0 / 16.0;
    float k = 0;
    for (int y = 4; y >= 0; --y) {
        for (int x = 0; x < 3; ++x) {
            if (k > v) {
                uBit.display.image.setPixelValue(2 - x, y, 0);
                uBit.display.image.setPixelValue(2 + x, y, 0);
            } else {
                uBit.display.image.setPixelValue(2 - x, y, 255);
                uBit.display.image.setPixelValue(2 + x, y, 255);
            }
            k += dv;
        }
    }
}

static void playMelody(const char melody[][NOTE_LEN], size_t len) {
    DMESG("Tune len: %d", len);

    // Starting from default values, optional octave & duration values are remembered
    int octave = 4;
    int durationMs = 4 * MS_PER_BPM;

    for (size_t i = 0; i < len; i++) {
        const char *note = melody[i];
        const char *note_char = &melody[i][0];
        int distanceFromA = 0;
        int frequency = 0;
        bool rest = false;

        // First process the note, as its distance from A
        switch (*note_char) {
            case 'A': distanceFromA = 0; break;
            case 'B': distanceFromA = 2; break;
            case 'C': distanceFromA = -9; break;
            case 'D': distanceFromA = -7; break;
            case 'E': distanceFromA = -5; break;
            case 'F': distanceFromA = -4; break;
            case 'G': distanceFromA = -2; break;
            case 'R': rest = true; break;
            default: target_panic(123); break;
        }

        // Then process the optional #/b modifiers and/or scale
        note_char++;
        while (*note_char != ':' && *note_char != '\0') {
            if (*note_char == '#') {
                distanceFromA++;
            } else if (*note_char == 'b') {
                distanceFromA--;
            } else if ((*note_char >= '0') && (*note_char <= '9')) {
                octave = (*note_char - '0');
            } else {
                target_panic(124);
            }
            note_char++;
        }

        // If an optional duration is present, calculate the delay in ms
        // Note, only a single digit implemented right now
        if (*note_char == ':') {
            note_char++;
            if ((*note_char < '0') || (*note_char > '9'))  target_panic(125);
            durationMs = atoi((const char*)note_char) * MS_PER_BPM;
        }

        // Calculate note frequency, or keep it as zero for a rest
        if (!rest) {
            float distanceFromA4 = (octave - 4) * 12 + distanceFromA;
            frequency = 440.0 * pow(2, distanceFromA4 / 12.0);
        }

        // Play the tone/rest for the calculated duration
        DMESG("%s -> f:%u, ms:%d", note, frequency, durationMs);
        if (frequency) {
            uBit.audio.virtualOutputPin.setAnalogPeriodUs(1000000 / frequency);
            uBit.audio.virtualOutputPin.setAnalogValue(127);
        }
        uBit.sleep(durationMs);
        uBit.audio.virtualOutputPin.setAnalogValue(0);

        // Short break between notes
        uBit.sleep(10);
    }
}

static void onButtonA(MicroBitEvent) {
    DMESG("Button A");
    uBit.audio.soundExpressions.playAsync("spring");
    uBit.display.print(HAPPY);
}

static void onButtonB(MicroBitEvent) {
    DMESG("Button B");
    uBit.audio.soundExpressions.playAsync("sad");
    uBit.display.print(SAD);
}

static void onButtonAB(MicroBitEvent) {
    DMESG("Button A+B");

    int lightLevel = uBit.display.readLightLevel();
    DMESG("Light level: %d", lightLevel);
    // Reset display mode to disable light level reading & avoid LED flicker
    uBit.display.setDisplayMode(DisplayMode::DISPLAY_MODE_BLACK_AND_WHITE);

    if (lightLevel > 50) {
        uBit.display.print(SUN);
        playMelody(MELODY_POWER_UP, TUNE_LEN(MELODY_POWER_UP));
    } else {
        uBit.display.print(MOON);
        playMelody(MELODY_POWER_DOWN, TUNE_LEN(MELODY_POWER_DOWN));
    }
}

static void onButtonLogo(MicroBitEvent) {
    DMESG("Button Logo");

    int sampleRate = 11000;
    static SplitterChannel *splitterChannel = uBit.audio.splitter->createChannel();
    splitterChannel->requestSampleRate( sampleRate );

    // Uncomment these two lines and comment out the *recording declaration after them to insert a low-pass-filter.
    // static LowPassFilter *lowPassFilter = new LowPassFilter(*splitterChannel, 0.812313f, false);
    // static StreamRecording *recording = new StreamRecording(*lowPassFilter);
    static StreamRecording *recording = new StreamRecording(*splitterChannel);

    static MixerChannel *channel = uBit.audio.mixer.addChannel(*recording, sampleRate);

    // uBit.audio.processor->setGain(0.08f);  // Default gain
    // uBit.audio.processor->setGain(0.16f);  // Double

    MicroBitAudio::requestActivation();
    channel->setVolume(75.0);
    uBit.audio.mixer.setVolume(1023);

    uBit.display.clear();
    uBit.audio.levelSPL->setUnit(LEVEL_DETECTOR_SPL_8BIT);

    recording->recordAsync();
    while (uBit.logo.isPressed() && recording->isRecording()) {
        int audioLevel = (int)uBit.audio.levelSPL->getValue();
        DMESG("Audio level: %d", audioLevel);
        plotBarGraph(audioLevel, 255);
        uBit.sleep(5);
    }
    // At this point either the logo has been released or the recording is done
    recording->stop();
    // Note: The CODAL_STREAM_IDLE_TIMEOUT_MS config has been set in the
    // codal.json file to reduce the time it takes for the microphone LED
    // to turn off after the recording is done.
    // This area is still being tweaked in CODAL and the codal.json config
    // should be removed in the future to use the CODAL default.
    uBit.display.clear();

    // If the recording is done but the logo is still pressed we want to
    // hold back the playback until the logo has been released
    while (uBit.logo.isPressed());

    recording->play();
    while (recording->isPlaying()) {
        uBit.sleep(20);
    }
    recording->erase();
}

static void onShake(MicroBitEvent) {
    DMESG("Shake");
    uBit.display.print(SILENT);
    uBit.sleep(400);

    // Sound Effect:
    // music.createSoundEffect(WaveShape.Sine, 3041, 3923, 59, 255, 500, SoundExpressionEffect.Warble, InterpolationCurve.Linear)
    // audio.SoundEffect(
    //     freq_start=3041, freq_end=3923,
    //     vol_start=59, vol_end=255,
    //     duration=500,
    //     waveform=audio.SoundEffect.WAVEFORM_SINE,
    //     fx=audio.SoundEffect.FX_WARBLE,
    //     shape=audio.SoundEffect.SHAPE_LINEAR,
    // )
    uBit.audio.soundExpressions.playAsync("002373041050001000392300001023010802050005000000000000000000000000000000");

    uBit.display.print(SINGING);
    uBit.sleep(400);
    uBit.display.print(SILENT);
}

static void onScreenDown(MicroBitEvent) {
    DMESG("Display Down");

    // Sound Effect:
    // music.createSoundEffect(WaveShape.Sine, 849, 1, 255, 0, 1000, SoundExpressionEffect.None, InterpolationCurve.Linear)
    // audio.SoundEffect(
    //     freq_start=849, freq_end=1,
    //     vol_start=255, vol_end=0,
    //     duration=1000,
    //     waveform=audio.SoundEffect.WAVEFORM_SINE,
    //     fx=audio.SoundEffect.FX_NONE,
    //     shape=audio.SoundEffect.SHAPE_LINEAR,
    // )
    uBit.audio.soundExpressions.play("010230849100001000000100000000012800000100240000000000000000000000000000");

    uBit.display.print(ASLEEP);
}

static void onStart() {
    uBit.audio.soundExpressions.playAsync("hello");
    uBit.display.print(HEART);
}

void out_of_box_experience() {
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_CLICK, onButtonAB);
    uBit.messageBus.listen(MICROBIT_ID_LOGO, MICROBIT_BUTTON_EVT_DOWN, onButtonLogo);
    uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_ACCELEROMETER_EVT_SHAKE, onShake);
    uBit.messageBus.listen(MICROBIT_ID_GESTURE, MICROBIT_ACCELEROMETER_EVT_FACE_DOWN, onScreenDown);

    onStart();

    //uBit.audio.rawSplitter->status |= DEVICE_COMPONENT_STATUS_SYSTEM_TICK;
    //uBit.audio.splitter->status |= DEVICE_COMPONENT_STATUS_SYSTEM_TICK;

    while (true) {
        uBit.sleep(1000);
    }
}
