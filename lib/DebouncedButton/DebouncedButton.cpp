#include "DebouncedButton.h"

DebouncedButton::DebouncedButton(uint8_t pin, bool pullup, unsigned long debounceMs)
  : _pin(pin), _pullup(pullup), _debounceDelay(debounceMs) {
  pinMode(_pin, pullup ? INPUT_PULLUP : INPUT);
  _currentState = digitalRead(_pin) == (_pullup ? LOW : HIGH);
  _lastStableState = _currentState;
  _lastChangeTime = millis();
}

void DebouncedButton::update() {
  bool rawState = digitalRead(_pin) == (_pullup ? LOW : HIGH);

  if (rawState != _lastStableState) {
    unsigned long now = millis();
    if ((now - _lastChangeTime) > _debounceDelay) {
      _lastStableState = rawState;

      if (_lastStableState) {
        _pressedFlag = true;
      } else {
        _releasedFlag = true;
      }
    }
  } else {
    _lastChangeTime = millis();  // reset timer if still stable
  }

  _currentState = _lastStableState;
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
