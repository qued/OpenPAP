#include "KY_040.h"

RotaryEncoder* RotaryEncoder::_instance = nullptr;

RotaryEncoder::RotaryEncoder(uint8_t clkPin, uint8_t dtPin, unsigned long debounceMicros)
    : _clkPin(clkPin), _dtPin(dtPin), _rawDelta(0), _position(0), _lastCLK(HIGH), _lastInterruptTime(0), _debounceMicros(debounceMicros) {}

void RotaryEncoder::begin() {
    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);

    _instance = this;  // set static pointer for ISR access

    attachInterrupt(digitalPinToInterrupt(_clkPin), isrHandler, CHANGE);
}

void IRAM_ATTR RotaryEncoder::isrHandler() {
    if (_instance) {
        _instance->handleInterrupt();
    }
}

void RotaryEncoder::handleInterrupt() {
    unsigned long now = micros();

    // Debounce: ignore if this interrupt fired too soon after the last
    if (now - _lastInterruptTime < _debounceMicros) return;
    _lastInterruptTime = now;

    bool clk = digitalRead(_clkPin);
    bool dt  = digitalRead(_dtPin);

    if (_lastCLK == HIGH && clk == LOW) {
        int8_t direction = (clk != dt) ? 1 : -1;
        _rawDelta += direction;
        _position += direction;
    }
    _lastCLK = clk;
}

RotaryEncoder::Direction RotaryEncoder::getDirection() {
    noInterrupts();
    int8_t delta = _rawDelta;
    _rawDelta = 0;
    interrupts();

    if (delta > 0) return CLOCKWISE;
    if (delta < 0) return COUNTERCLOCKWISE;
    return NONE;
}

int RotaryEncoder::getDelta() {
    noInterrupts();
    int8_t delta = _rawDelta;
    _rawDelta = 0;
    interrupts();
    return delta;
}

int RotaryEncoder::getPosition() const {
    return _position;
}
