#pragma once

#include <Arduino.h>

class RotaryEncoder {
public:
    enum Direction { NONE, CLOCKWISE, COUNTERCLOCKWISE };

    RotaryEncoder(uint8_t clkPin, uint8_t dtPin, unsigned long debounceMicros=2000);
    void begin();               // call in setup()
    Direction getDirection();   // returns and clears last direction
    int getDelta();             // net movement since last check
    int getPosition() const;    // total movement since boot

private:
    static void isrHandler();   // static ISR trampoline
    void handleInterrupt();     // actual handler

    uint8_t _clkPin, _dtPin;
    volatile int8_t _rawDelta;
    volatile int _position;

    static RotaryEncoder* _instance;  // for static ISR
    volatile bool _lastCLK; // new logic
    volatile unsigned long _lastInterruptTime = 0;
    // debounceMicros: reject changes faster than this many microseconds apart (default: 2000 µs)
    const unsigned long _debounceMicros;
};
