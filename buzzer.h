#ifndef BUZZER_H
#define BUZZER_H

typedef struct note_t
{
    uint16_t pitch;
    u8 duration; // inverse of duration; e.g. 2 here is 1/2th, 8 here is 1/8th.
} note_t;

typedef struct tune_t
{
    const note_t *notes;
    uint8_t n_notes;
} tune_t;

typedef struct buzzer_t
{
    byte pin;
    const tune_t *tune;
    unsigned long barSizeMillis;
    int16_t curNote;
    unsigned long curNoteStartedMicros;
} buzzer_t;

void buzzerInit(volatile buzzer_t *handler, byte pin, const tune_t *tune, unsigned long barSizeMs);
void buzzerReset(volatile buzzer_t *handler);
void buzzerPlay(volatile buzzer_t *handler);
bool buzzerDonePlaying(const volatile buzzer_t *handler);
#endif