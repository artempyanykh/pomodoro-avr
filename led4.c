#include <Arduino.h>
#include "led4.h"

#define NUM_DIGITS 4

// How long in microseconds we flash a single led segment during each cycle
#define FLASH_DUR_US 1000UL
// The duration in microseconds of the cycle which consists of flashing all leds and dimmin all leds
#define CYCLE_DUR_US 15000UL

const byte segmentCodes[] = {
    0b00001000, // left-most segment
    0b00000100,
    0b00000010,
    0b00000001, // right-most segment
};

typedef struct led4_t
{
    uint8_t dataPin;
    uint8_t latchPin;
    uint8_t clockPin;
    byte *data; // 4 byte array, guaranteed by init function
    uint8_t curSegment;
    unsigned long cycleStartTs;
    unsigned long curSegmentTs;
    bool isSegmentOn;
} led4_t;

void led4Init(led4_t *handle, uint8_t dataPin, uint8_t latchPin, uint8_t clockPin, byte data[4])
{
    // Init handle elements
    handle->dataPin = dataPin;
    handle->latchPin = latchPin;
    handle->clockPin = clockPin;
    handle->data = data;
    resetHandle(handle);

    // Init arduino pins
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
    pinMode(latchPin, OUTPUT);
    digitalWrite(latchPin, LOW);
    pinMode(clockPin, OUTPUT);
    digitalWrite(clockPin, LOW);
}

void led4Refresh(led4_t *handle)
{
    // millis resolution is not enough here which leads to blinking and unevenness
    // we really need micros here
    unsigned long curTs = micros();

    // If for some reason the led gets stuck, reset it
    if (curTs - handle->cycleStartTs > 2 * CYCLE_DUR_US)
    {
        Serial.println("LED4 got stuck");
        resetHandle(handle);
    }

    if (!handle->isSegmentOn)
    {
        if (curTs > handle->cycleStartTs + CYCLE_DUR_US)
        {
            flashDigit(handle);
            handle->isSegmentOn = true;
            handle->curSegmentTs = curTs;

            if (handle->curSegment == 0)
            {
                handle->cycleStartTs = curTs;
            }
        }
    }
    else
    {
        if (curTs > handle->curSegmentTs + FLASH_DUR_US)
        {
            led4Dim(handle);
            handle->isSegmentOn = false;
            handle->curSegment = (handle->curSegment + 1) % NUM_DIGITS;
        }
    }
}

// Helper functions

void led4Dim(const led4_t *handle)
{
    for (int i = 0; i < NUM_DIGITS; i++)
    {
        digitalWrite(handle->latchPin, LOW);
        shiftOut(handle->dataPin, handle->clockPin, MSBFIRST, 0);
        shiftOut(handle->dataPin, handle->clockPin, LSBFIRST, 0);
        digitalWrite(handle->latchPin, HIGH);
    }
}

void flashDigit(const led4_t *handle)
{
    digitalWrite(handle->latchPin, LOW);
    shiftOut(handle->dataPin, handle->clockPin, MSBFIRST, segmentCodes[handle->curSegment]);
    shiftOut(handle->dataPin, handle->clockPin, LSBFIRST, handle->data[handle->curSegment]);
    digitalWrite(handle->latchPin, HIGH);
}

void resetHandle(led4_t *handle)
{
    handle->curSegment = 0;
    handle->cycleStartTs = 0;
    handle->curSegmentTs = 0;
    handle->isSegmentOn = false;
}