#ifndef DEBOUNCED_BUTTON_H
#define DEBOUNCED_BUTTON_H

#include <Arduino.h>

class DebouncedButton {
public:
  DebouncedButton(uint8_t pin, bool pullup = true, unsigned long debounceMs = 50);

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

#endif
