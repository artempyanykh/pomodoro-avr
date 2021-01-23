#include <pitches.h>

#include "led4.h"
#include "button.h"
#include "buzzer.h"

#define CLOCK_DATA_PIN 8
#define CLOCK_LATCH_PIN 7
#define CLOCK_CLOCK_PIN 4

#define START_BUTTON_PIN 2
#define PRESS_LATENCY 200UL
#define BUZZER_PIN 12

#define BAUD_RATE 115200

#define DEBUG_PIN 10

//
// Setup clock LED
//
led4_t clockLed;
byte display[4] = {charToCode('-'), charToCode('-'), charToCode('-'), charToCode('-')};
void setDisplay(byte d1, byte d2, byte d3, byte d4)
{
    display[0] = d1;
    display[1] = d2;
    display[2] = d3;
    display[3] = d4;
}

//
// Setup start button and control vars
//
volatile button_t startButton;

volatile bool isTimerSet = false;
volatile bool isTimerCompleted = false;
volatile long timerSetTs = 0L;

long timerDurationSec = 25 * 60; // 25min
uint16_t timersDone = 0;

//
// Setup buzzer to play "timer done" tune
//
const note_t marioFlagPole[] = {
    {NOTE_G3, 4},
    {NOTE_C4, 4},
    {NOTE_E4, 4},
    {NOTE_G4, 4},
    {NOTE_C5, 4},
    {NOTE_E5, 4},
    //
    {NOTE_G5, 2},
    {NOTE_E5, 2},
    //
    {NOTE_GS3, 4},
    {NOTE_C4, 4},
    {NOTE_DS4, 4},
    {NOTE_GS4, 4},
    {NOTE_C5, 4},
    {NOTE_DS5, 4},
    //
    {NOTE_GS5, 2},
    {NOTE_DS5, 2},
    //
    {NOTE_AS3, 4},
    {NOTE_D4, 4},
    {NOTE_F4, 4},
    {NOTE_AS4, 4},
    {NOTE_D5, 4},
    {NOTE_F5, 4},
    //
    {NOTE_AS5, 2},
    {NOTE_AS5, 4},
    {NOTE_AS5, 4},
    {NOTE_AS5, 4},
    //
    {NOTE_C6, 1},
};
const tune_t marioFlagPoleTune = {marioFlagPole, sizeof(marioFlagPole) / sizeof(marioFlagPole[0])};
volatile buzzer_t timerCompletedBuzz; // init in setup

//
// Helper functions / interrupt handlers
//

void toggleTimer()
{
    long curTs = millis();
    if (isTimerCompleted)
    {
        isTimerSet = false;
        isTimerCompleted = false;
    }
    else
    {
        isTimerSet = !isTimerSet;
    }

    timerSetTs = curTs;

    buzzerReset(&timerCompletedBuzz);
}

void refreshStartButton()
{
    buttonRefresh(&startButton);
}

//
// Setup and main loop
//

void setup()
{
    led4Init(&clockLed, CLOCK_DATA_PIN, CLOCK_LATCH_PIN, CLOCK_CLOCK_PIN, display);

    buttonInit(&startButton, START_BUTTON_PIN, true);
    attachInterrupt(digitalPinToInterrupt(START_BUTTON_PIN), refreshStartButton, CHANGE);

    buzzerInit(&timerCompletedBuzz, BUZZER_PIN, &marioFlagPoleTune, 700UL);

    pinMode(DEBUG_PIN, INPUT_PULLUP);
    bool isDebugOn = digitalRead(DEBUG_PIN) == LOW;
    if (isDebugOn)
    {
        timerDurationSec = 3;
    }

    Serial.begin(BAUD_RATE);
}

void loop()
{
    button_state_t startButtonState = buttonStateRead(&startButton);
    if (startButtonState == CLICK)
    {
        toggleTimer();
    }
    else if (startButtonState == LONG_PRESS)
    {
        timersDone = 0;
    }

    long curTime = millis();
    if (isTimerSet)
    {
        long elapsedHalfSec = (curTime - timerSetTs) / 500;
        bool isHalfSec = elapsedHalfSec % 2 == 0;
        long elapsedSec = elapsedHalfSec / 2;
        long timerLeftSec = timerDurationSec - elapsedSec;

        if (timerLeftSec > 0)
        {
            byte minutesHigh = timerLeftSec / 60 / 10;
            byte minutesLow = timerLeftSec / 60 % 10;
            byte secondsHigh = timerLeftSec % 60 / 10;
            byte secondsLow = timerLeftSec % 10;

            setDisplay(
                digitToCode(minutesHigh),
                digitToCode(minutesLow),
                digitToCode(secondsHigh),
                digitToCode(secondsLow) | isHalfSec);
        }
        else
        {
            isTimerCompleted = true;
            isTimerSet = !isTimerSet;
            timersDone++;
        }
    }

    if (isTimerCompleted)
    {
        if (curTime % 100 == 0)
        {
            setDisplay(
                1 << (curTime / 100 % 6) << 2,
                (1 << 7) >> (curTime / 100 % 6),
                1 << (curTime / 100 % 6) << 2,
                (1 << 7) >> (curTime / 100 % 6));
        }
        buzzerPlay(&timerCompletedBuzz);
        if (buzzerDonePlaying(&timerCompletedBuzz))
        {
            isTimerCompleted = false;
        }
    }
    else if (!isTimerSet)
    {
        byte segment0 = timersDone / 1000 % 10;
        segment0 = segment0 == 0 ? blankDigit() : digitToCode(segment0);
        byte segment1 = timersDone / 100 % 10;
        segment1 = segment0 == 0 && segment1 == 0 ? blankDigit() : digitToCode(segment1);
        byte segment2 = timersDone / 10 % 10;
        segment2 = segment0 == 0 && segment1 == 0 && segment2 == 0 ? blankDigit() : digitToCode(segment2);
        byte segment3 = timersDone % 10;
        segment3 = digitToCode(segment3);
        setDisplay(segment0, segment1, segment2, segment3);
    }

    led4Refresh(&clockLed);
}