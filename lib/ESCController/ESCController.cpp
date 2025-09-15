#include "ESCController.h"

ESCController::ESCController(uint8_t pwmPin, int minPulse, int maxPulse)
  : _pwmPin(pwmPin), _minPulse(minPulse), _maxPulse(maxPulse), _currentPulse(minPulse) {}

void ESCController::begin() {
  _servo.setPeriodHertz(50); // Standard servo PWM frequency
  _servo.attach(_pwmPin, _minPulse, _maxPulse);
}

void ESCController::arm() {
  stop();
  delay(3000); // Give ESC time to recognize idle
}

void ESCController::stop() {
  _currentPulse = _minPulse;
  _servo.writeMicroseconds(_currentPulse);
  delay(200);
}

void ESCController::setThrottle(float value) {
  value = constrain(value, 0.0, 1.0);
  _currentPulse = _minPulse + (int)((_maxPulse - _minPulse) * value);
  _servo.writeMicroseconds(_currentPulse);
}

void ESCController::setThrottleMicroseconds(int us) {
  _currentPulse = constrain(us, _minPulse, _maxPulse);
  _servo.writeMicroseconds(_currentPulse);
}

int ESCController::getThrottleMicroseconds() const {
  return _currentPulse;
}

float ESCController::getThrottle() const {
  return (float)(_currentPulse - _minPulse) / (_maxPulse - _minPulse);
}
