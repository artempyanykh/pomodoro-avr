# Pomodoro timer on Arduino Uno/ATmega328P

## Overview of the functionality
1. The 4-digit 7-segment LED display shows 
    - the number of pomodoros completed while in "stand-by" mode,
    - the current pomodoro timer while in "timer-on" mode.
2. The buzzer plays a "Mario Flagpole" tune when the pomodoro timer has completed.
3. The button has the following functionality attached:
    1. Long-press for 2 seconds resets the completed pomodoro timer.
    2. Click generally toggles the timer.
    3. When the tune is playing, clicking the button stops the tune.

## Components

1. ATmega328P micro-controller (e.g. Arduino Uno).
2. [5641AS LED display](http://www.xlitx.com/datasheet/5641AS.pdf).
3. 2 x [SN74HC595N](https://www.ti.com/lit/ds/scls041i/scls041i.pdf) serial-to-parallel shift register.
4. Passive buzzer.
5. Push button switch.
6. 16MHz crystal.
6. Resistors, transistors, capacitors.