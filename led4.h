#ifndef LED4_H
#define LED4_H

typedef struct led4_t led4_t;

void led4Init(led4_t *handle, uint8_t dataPin, uint8_t latchPin, uint8_t clockPin, byte data[4]);

void led4Refresh(led4_t *handle);

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

#define BLANK 0b00000000
#define NA_CHAR 0b00000010
#define H_CHAR 0b01101110
#define I_CHAR 0b00001100

#endif