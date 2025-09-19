#pragma once

#include <Arduino.h>

class DebouncedButton {
public:
  DebouncedButton(const uint8_t pin, bool pullup = true, const unsigned long debounceMs = 50);

  void update();                  // Call this regularly in loop()
  bool isPressed() const;        // Current debounced state
  bool wasPressed();             // Returns true *once* when newly pressed
  bool wasReleased();            // Returns true *once* when newly released

private:
  uint8_t _pin;
  bool _pullup;
  unsigned long _debounceDelay;
  bool _currentState;
  bool _lastStableState;
  unsigned long _lastChangeTime;

  bool _pressedFlag = false;
  bool _releasedFlag = false;
};
