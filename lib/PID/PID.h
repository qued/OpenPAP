#pragma once

#include <Arduino.h>

class PID {
public:
    float errorSum;

    PID(float* input, float* output, float setpoint, float Kp, float Ki, float Kd, float outMin, float outMax);
    void setTunings(float newKp, float newKi, float newKd);
    void compute(float dt); 
    void reset();

private:
    float _Kp, _Ki, _Kd;
    const float* _input;
    float* const _output;
    float _setpoint;
    float _lastInput;
    float _outMin, _outMax;
};
