#include "DebouncedButton.h"

DebouncedButton::DebouncedButton(const uint8_t pin, bool pullup, const unsigned long debounceMs)
    : _pin(pin), _pullup(pullup), _debounceDelay(debounceMs) {
    pinMode(_pin, pullup ? INPUT_PULLUP : INPUT);
    _currentState = digitalRead(_pin) == (_pullup ? LOW : HIGH);
    _lastStableState = _currentState;
    _lastChangeTime = millis();
}

void DebouncedButton::update() {
    bool rawState = digitalRead(_pin) == (_pullup ? LOW : HIGH);
    unsigned long now = millis();

    if (rawState != _currentState) {
        _lastChangeTime = now;
        _currentState = rawState;
    }

    if ((now - _lastChangeTime) > _debounceDelay) {
        if (_currentState != _lastStableState) {
            _lastStableState = _currentState;
            if (_lastStableState) {
                _pressedFlag = true;
            } else {
                _releasedFlag = true;
            }
        }
    }
}

bool DebouncedButton::isPressed() const {
  return _currentState;
}

bool DebouncedButton::wasPressed() {
  if (_pressedFlag) {
    _pressedFlag = false;
    return true;
  }
  return false;
}

bool DebouncedButton::wasReleased() {
  if (_releasedFlag) {
    _releasedFlag = false;
    return true;
  }
  return false;
}
