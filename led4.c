#include <Arduino.h>
#include <assert.h>
#include "led4.h"

#define NUM_DIGITS 4

// How long in microseconds we flash a single led segment during each cycle
#define FLASH_DUR_US 1000UL
// The duration in microseconds of the cycle which consists of flashing all leds and dimmin all leds
#define CYCLE_DUR_US 14000UL

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

const byte digitCodes[] = {
    //A B C D E F G DP
    0b11111100, // 0
    0b01100000, // 1
    0b11011010, // 2
    0b11110010, // 3
    0b01100110, // 4
    0b10110110, // 5
    0b10111110, // 6
    0b11100000, // 7
    0b11111110, // 8
    0b11110110, // 9
};

byte digitToCode(byte digit)
{
    assert(digit >= 0 && digit <= 9);
    return digitCodes[digit];
}

byte charToCode(char ch)
{
    switch (ch)
    {
    case 'a':
    case 'A':
        return 0b11101110;
    case 'b':
    case 'B':
        return 0b00111110;
    case 'c':
    case 'C':
        return 0b10011100;
    case 'd':
    case 'D':
        return 0b01111010;
    case 'e':
    case 'E':
        return 0b10011110;
    case 'f':
    case 'F':
        return 0b10001110;
    case 'g':
    case 'G':
        return 0b11110110;
    case 'h':
    case 'H':
        return 0b00101110;
    case 'i':
    case 'I':
        return 0b01100000;
    case 'j':
    case 'J':
        return 0b01110000;
    case 'k':
    case 'K':
        return 0b01101110;
    case 'l':
    case 'L':
        return 0b00011100;
    case 'o':
    case 'O':
        return 0b11111100;
    case 'p':
    case 'P':
        return 0b11001110;
    case 's':
    case 'S':
        return 0b10110110;
    case 'u':
    case 'U':
        return 0b01111100;
    case 'y':
    case 'Y':
        return 0b01100110;
    case 'z':
    case 'Z':
        return 0b1101101;
    case '-':
        return 0b00000010;
    default:
        return 0b00000001; // just light the dot to signal the char is not available
    }
}

byte blankDigit()
{
    return 0;
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