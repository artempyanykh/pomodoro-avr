#ifndef BUTTON_H
#define BUTTON_H

typedef enum button_internal_state_t
{
    UP,
    DOWN,
} button_internal_state_t;

typedef struct button_t
{
    byte pin;
    bool softwareDebounce;
    button_internal_state_t internal_state;
    unsigned long lastDownTs;      // in millis
    unsigned long lastUpTs;        // in millis
    unsigned long stateLastReadTs; // in millis
} button_t;

typedef enum button_state_t
{
    NO_STATE,
    LONG_PRESS,
    CLICK
} button_state_t;

void buttonInit(volatile button_t *handler, byte pin, bool softwareDebounde);
void buttonRefresh(volatile button_t *handler);
button_state_t buttonStateRead(volatile button_t *handler);
#endif