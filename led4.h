#ifndef LED4_H
#define LED4_H

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

void led4Init(led4_t *handle, uint8_t dataPin, uint8_t latchPin, uint8_t clockPin, byte data[4]);

void led4Refresh(led4_t *handle);

byte digitToCode(byte digit);
byte charToCode(char ch);
byte blankDigit();

#endif