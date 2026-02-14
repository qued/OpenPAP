#include "ESC.h"

ESC::ESC(uint8_t pwmPin, int minPulse, int maxPulse)
    : _pwmPin(pwmPin), _minPulse(minPulse), _maxPulse(maxPulse), _currentPulse(minPulse) {}

void ESC::begin() {
    _servo.setPeriodHertz(50); // Standard servo PWM frequency
    _servo.attach(_pwmPin, _minPulse, _maxPulse);
}

void ESC::arm() {
    stop();
    delay(3000); // Give ESC time to recognize idle
}

void ESC::stop() {
    _currentPulse = _minPulse;
    _servo.writeMicroseconds(_currentPulse);
    delay(200);
}

void ESC::setThrottle(float value) {
    value = constrain(value, 0.0, 1.0);
    _currentPulse = _minPulse + (int)((_maxPulse - _minPulse) * value);
    _servo.writeMicroseconds(_currentPulse);
}

void ESC::setThrottleMicroseconds(int us) {
    _currentPulse = constrain(us, _minPulse, _maxPulse);
    _servo.writeMicroseconds(_currentPulse);
}

int ESC::getThrottleMicroseconds() const {
    return _currentPulse;
}

float ESC::getThrottle() const {
    return (float)(_currentPulse - _minPulse) / (_maxPulse - _minPulse);
}
