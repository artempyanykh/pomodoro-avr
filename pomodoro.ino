#include <pitches.h>
#include "led4.h"

#define DISP_VAL 8
#define DISP_LATCH 7
#define DISP_CLOCK 4

#define BUTTON 2
#define PRESS_LATENCY 200UL
#define BUZZ 12

led4_t clockLed;
byte display[4] = {NA_CHAR, NA_CHAR, NA_CHAR, NA_CHAR};

volatile bool isTimerOn = false;
volatile bool shouldPlayTune = false;
volatile long timerSetTs = 0L;

const long timerDurationSec = 25 * 60; // 25min
uint16_t timersDone = 0;

void setDisplay(byte d1, byte d2, byte d3, byte d4)
{
    display[0] = d1;
    display[1] = d2;
    display[2] = d3;
    display[3] = d4;
}

struct note
{
    uint16_t pitch;
    u8 duration; // inverse of duration; e.g. 2 here is 1/2th, 8 here is 1/8th.
};

struct tune
{
    const struct note *notes;
    uint8_t n_notes;
};

struct player
{
    const struct tune *tune;
    long barSizeMillis;
    int16_t curNote;
    long curNoteStartedTs;
};

const note marioFlagPole[] = {
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

const struct tune marioFlagPoleTune = {marioFlagPole, sizeof(marioFlagPole) / sizeof(marioFlagPole[0])};

struct player timerDonePlayer = {&marioFlagPoleTune, 700L, 0, 0};

void play(struct player *player, long curTs)
{
    if (!shouldPlayTune)
    {
        noTone(BUZZ);
        return;
    }

    if (player->curNote < 0)
    {
        player->curNote = 0;
        player->curNoteStartedTs = curTs;
        const struct note *newNote = &player->tune->notes[player->curNote];
        long newNoteDurationMillis = player->barSizeMillis / newNote->duration;
        tone(BUZZ, newNote->pitch, newNoteDurationMillis);
    }

    if (player->curNote >= player->tune->n_notes)
    {
        noTone(BUZZ);
        shouldPlayTune = false;
        return;
    }

    const struct note *curNote = &player->tune->notes[player->curNote];
    long noteDurationMillis = player->barSizeMillis / curNote->duration;
    long notePlayedMillis = curTs - player->curNoteStartedTs;
    if (notePlayedMillis > noteDurationMillis)
    {
        player->curNote++;
        player->curNoteStartedTs = curTs;
        const struct note *newNote = &player->tune->notes[player->curNote];
        long newNoteDurationMillis = player->barSizeMillis / newNote->duration;
        tone(BUZZ, newNote->pitch, newNoteDurationMillis);
    }
}

void startTimer()
{
    noInterrupts();

    long curTs = millis();
    if (curTs < timerSetTs + PRESS_LATENCY)
    {
        return;
    }

    if (shouldPlayTune)
    {
        isTimerOn = false;
        shouldPlayTune = false;
    }
    else
    {
        isTimerOn = !isTimerOn;
    }

    timerSetTs = curTs;

    timerDonePlayer.curNote = -1;
    timerDonePlayer.curNoteStartedTs = 0;

    interrupts();
}

void setup()
{
    led4Init(&clockLed, DISP_VAL, DISP_LATCH, DISP_CLOCK, display);

    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON), startTimer, FALLING);

    pinMode(BUZZ, OUTPUT);
    noTone(BUZZ);

    timerDonePlayer.curNote = -1;
    timerDonePlayer.curNoteStartedTs = 0;

    Serial.begin(9600);
}

void loop()
{
    long curTime = millis();

    if (isTimerOn)
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
                digitCodes[minutesHigh],
                digitCodes[minutesLow],
                digitCodes[secondsHigh],
                digitCodes[secondsLow] | isHalfSec);
        }
        else
        {
            shouldPlayTune = true;
            isTimerOn = !isTimerOn;
            timersDone++;
        }
    }

    if (shouldPlayTune)
    {
        if (curTime % 100 == 0)
        {
            setDisplay(
                1 << (curTime / 100 % 6) << 2,
                (1 << 7) >> (curTime / 100 % 6),
                1 << (curTime / 100 % 6) << 2,
                (1 << 7) >> (curTime / 100 % 6));
        }
        play(&timerDonePlayer, curTime);
    }
    else if (!isTimerOn)
    {
        byte segment0 = timersDone / 1000 % 10;
        segment0 = segment0 == 0 ? BLANK : digitCodes[segment0];
        byte segment1 = timersDone / 100 % 10;
        segment1 = segment1 == 0 ? BLANK : digitCodes[segment1];
        byte segment2 = timersDone / 10 % 10;
        segment2 = segment2 == 0 ? BLANK : digitCodes[segment2];
        byte segment3 = timersDone % 10;
        segment3 = digitCodes[segment3];
        setDisplay(segment0, segment1, segment2, segment3);
    }

    led4Refresh(&clockLed);
}