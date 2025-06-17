#include "Tests.h"

#include <cctype>

// Based off: https://github.com/WarwickDCSiLab/arduino-morse-code/blob/master/morse.ino

// Morse code timing constants
// Time differences come from the international standard
constexpr int FRAME_LENGTH = 280;
constexpr int MAX_FRAME_LENGTH = 500;
constexpr int MIN_FRAME_LENGTH = 10;

static_assert(FRAME_LENGTH >= MIN_FRAME_LENGTH);
static_assert(FRAME_LENGTH <= MAX_FRAME_LENGTH);

#define DOT_TIME FRAME_LENGTH
#define DASH_TIME DOT_TIME*3
#define INNER_SEP_TIME DOT_TIME*1
#define LETTER_SEP_TIME DOT_TIME*3
#define WORD_SEP_TIME DOT_TIME*7

// Morse symbol values
constexpr int term = 0;
constexpr int dit = 1;
constexpr int dah = 2;

// Morse letter definitions
constexpr int A[] = { dit, dah, term };
constexpr int B[] = { dah, dit, dit, dit, term };
constexpr int C[] = { dah, dit, dah, dit, term };
constexpr int D[] = { dah, dit, dit, term };
constexpr int E[] = { dit, term };
constexpr int F[] = { dit, dit, dah, dit, term };
constexpr int G[] = { dah, dah, dit, term };
constexpr int H[] = { dit, dit, dit, dit, term };
constexpr int I[] = { dit, dit, term };
constexpr int J[] = { dit, dah, dah, dah, term };
constexpr int K[] = { dah, dit, dah, term };
constexpr int L[] = { dit, dah, dit, dit, term };
constexpr int M[] = { dah, dah, term };
constexpr int N[] = { dah, dit, term };
constexpr int O[] = { dah, dah, dah, term };
constexpr int P[] = { dit, dah, dah, dit, term };
constexpr int Q[] = { dah, dah, dit, dah, term };
constexpr int R[] = { dit, dah, dit, term };
constexpr int S[] = { dit, dit, dit, term };
constexpr int T[] = { dah, term };
constexpr int U[] = { dit, dit, dah, term };
constexpr int V[] = { dit, dit, dit, dah, term };
constexpr int W[] = { dit, dah, dah, term };
constexpr int X[] = { dah, dit, dit, dah, term };
constexpr int Y[] = { dah, dit, dah, dah, term };
constexpr int Z[] = { dah, dah, dit, dit, term };

const int* morse_code_letters[] =
{
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
};

const char* morse_code_numbers[] =
{
    "ZERO",
    "ONE",
    "TWO",
    "THREE",
    "FOUR",
    "FIVE",
    "SIX",
    "SEVEN",
    "EIGHT",
    "NINE"
};

constexpr int C5 = 523;

// From source/samples/AudioTest.cpp
static void analogPitch(int frequency) {
    constexpr uint8_t pitchVolume = 0xA0;

    if (frequency <= 0 || pitchVolume == 0) {
        uBit.audio.virtualOutputPin.setAnalogValue(0);
    } else {
        // I don't understand the logic of this value.
        // It is much louder on the real pin.
        int v = 1 << (pitchVolume >> 5);
        // If you flip the order of these they crash on the real pin with E030.
        uBit.audio.virtualOutputPin.setAnalogValue(v);
        uBit.audio.virtualOutputPin.setAnalogPeriodUs(1000000/frequency);
    }
}

static void on()
{
    analogPitch(C5);
    uBit.io.col1.setDigitalValue(0);
    uBit.io.col2.setDigitalValue(0);
    uBit.io.col3.setDigitalValue(0);
    uBit.io.col4.setDigitalValue(0);
    uBit.io.col5.setDigitalValue(0);
}

static void off()
{
    uBit.io.col1.setDigitalValue(1);
    uBit.io.col2.setDigitalValue(1);
    uBit.io.col3.setDigitalValue(1);
    uBit.io.col4.setDigitalValue(1);
    uBit.io.col5.setDigitalValue(1);

    uBit.audio.virtualOutputPin.setAnalogValue(0);
}

// Outputs a single letter of a word from array constants
static void blinkLetter(const int* morse)
{
    int i = 0;
    while (morse[i] != term)
    {
        switch (morse[i])
        {
        case dit:
            on();
            uBit.sleep(DOT_TIME);
            off();
            uBit.sleep(INNER_SEP_TIME);
            break;
        case dah:
            on();
            uBit.sleep(DASH_TIME);
            off();
            uBit.sleep(INNER_SEP_TIME);
            break;
        }
        i++;
    }

    uBit.sleep(LETTER_SEP_TIME);
}

// Convert single digit to Morse Code
static void blinkDigit(int n)
{
    const char* letters = morse_code_numbers[n];
    const size_t letters_length = strlen(letters);
    for(size_t i = 0; i < letters_length; i++)
    {
        blinkLetter(morse_code_letters[letters[i]-'A']);
    }
    uBit.sleep(WORD_SEP_TIME);
}

void morse_code()
{
    const char message[] = "LANCASHIRE";

    uBit.display.disable();
    uBit.io.row1.setDigitalValue(1);
    uBit.io.row2.setDigitalValue(1);
    uBit.io.row3.setDigitalValue(1);
    uBit.io.row4.setDigitalValue(1);
    uBit.io.row5.setDigitalValue(1);

    while (true)
    {
        for (char ch : message)
        {
            if (isdigit(ch))
            {
                blinkDigit(ch - '0');
            }
            else if (isalpha(ch))
            {
                ch = (char)toupper(ch);
                blinkLetter(morse_code_letters[ch - 'A']);
            }
            else
            {
                // Skip
            }
        }

        // Wait before starting again
        uBit.sleep(5 * 1000);
    }
}
