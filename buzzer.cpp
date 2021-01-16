#include <Arduino.h>
#include "buzzer.h"

void buzzerInit(volatile buzzer_t *handler, byte pin, const tune_t *tune, unsigned long barSizeMs)
{
    handler->pin = pin;
    pinMode(pin, OUTPUT);
    noTone(pin);

    handler->tune = tune;
    handler->barSizeMillis = barSizeMs;
    buzzerReset(handler);
}

void buzzerReset(volatile buzzer_t *handler)
{
    noTone(handler->pin);
    handler->curNote = -1;
    handler->curNoteStartedMicros = 0;
}

void startCurrentTone(volatile const buzzer_t *handler);

void buzzerPlay(volatile buzzer_t *handler)
{
    unsigned long curTsMicros = micros();

    if (handler->curNote < 0)
    {
        handler->curNote = 0;
        handler->curNoteStartedMicros = curTsMicros;

        startCurrentTone(handler);
    }

    if (handler->curNote >= handler->tune->n_notes)
    {
        return;
    }

    const note_t *curNote = &handler->tune->notes[handler->curNote];
    unsigned long noteDurationMillis = handler->barSizeMillis / curNote->duration;
    unsigned long notePlayedMillis = (curTsMicros - handler->curNoteStartedMicros) / 1000;

    if (notePlayedMillis > noteDurationMillis)
    {
        handler->curNote++;
        handler->curNoteStartedMicros = curTsMicros;

        startCurrentTone(handler);
    }
}

bool buzzerDonePlaying(const volatile buzzer_t *handler)
{
    return handler->curNote >= handler->tune->n_notes;
}

void startCurrentTone(volatile const buzzer_t *handler)
{
    const note_t *newNote = &handler->tune->notes[handler->curNote];
    long newNoteDurationMillis = handler->barSizeMillis / newNote->duration;
    tone(handler->pin, newNote->pitch, newNoteDurationMillis);
}