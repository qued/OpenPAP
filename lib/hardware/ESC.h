#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "IMotor.h"

class ESC : public IMotor {
public:
    ESC(uint8_t pwmPin, int minPulse = 1000, int maxPulse = 2000);

    void begin() override;
    void arm() override;
    void stop() override;

    void setThrottle(float value) override;
    float getThrottle() const override;

    // Hardware-specific API (optional, but allowed)
    void setThrottleMicroseconds(int us);
    int getThrottleMicroseconds() const;

private:
    uint8_t _pwmPin;
    int _minPulse;
    int _maxPulse;
    int _currentPulse;

    Servo _servo;
};
