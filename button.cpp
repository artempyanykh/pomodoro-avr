#include <Arduino.h>
#include "button.h"

#define DEBOUNCE_DELAY_MS 50UL
#define LONG_PRESS_DELAY_MS 2000UL

void buttonInit(volatile button_t *handler, byte pin, bool softwareDebounce)
{

    handler->pin = pin;
    handler->softwareDebounce = softwareDebounce;
    pinMode(pin, INPUT_PULLUP);
    handler->lastDownTs = 0;
    handler->lastUpTs = 0;
    handler->stateLastReadTs = 0;

    buttonRefresh(handler);
}

void buttonRefresh(volatile button_t *handler)
{
    unsigned long curTs = millis();
    if (!handler->softwareDebounce || curTs - buttonLastChangeTs(handler) > DEBOUNCE_DELAY_MS)
    {
        int state = digitalRead(handler->pin) == LOW ? DOWN : UP;
        if (handler->internal_state == UP && state == DOWN)
        {
            handler->internal_state = DOWN;
            handler->lastDownTs = curTs;
        }
        else if (handler->internal_state == DOWN && state == UP)
        {
            handler->internal_state = UP;
            handler->lastUpTs = curTs;
        }
    }
}

button_state_t buttonStateRead(volatile button_t *handler)
{
    if (handler->lastDownTs == 0 && handler->lastUpTs == 0)
    {
        return NO_STATE;
    }
    button_state_t preliminary = NO_STATE;
    unsigned long stateTs = 0;
    unsigned long curTs = millis();

    if (handler->internal_state == DOWN && curTs - handler->lastDownTs > LONG_PRESS_DELAY_MS)
    {
        preliminary = LONG_PRESS;
        stateTs = handler->lastDownTs;
    }

    if (handler->internal_state == UP && handler->lastUpTs > handler->lastDownTs && handler->lastUpTs - handler->lastDownTs < LONG_PRESS_DELAY_MS)
    {
        preliminary = CLICK;
        stateTs = handler->lastUpTs;
    }

    if (handler->stateLastReadTs == stateTs)
    {
        return NO_STATE;
    }
    else
    {
        handler->stateLastReadTs = stateTs;
        return preliminary;
    }
}

unsigned long buttonLastChangeTs(const volatile button_t *handler)
{
    return max(handler->lastDownTs, handler->lastUpTs);
}