#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

class ESC {
public:
    ESC(uint8_t pwmPin, int minPulse = 1000, int maxPulse = 2000);

    void begin();
    void arm();
    void stop();
    void setThrottle(float value);      // 0.0 - 1.0
    void setThrottleMicroseconds(int us);
    int getThrottleMicroseconds() const;
    float getThrottle() const;

private:
    uint8_t _pwmPin;
    int _minPulse;
    int _maxPulse;
    int _currentPulse;
    Servo _servo;
};
