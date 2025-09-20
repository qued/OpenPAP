#pragma once

#include <Arduino.h>

class PID {
private:
    float Kp, Ki, Kd;
    float *input, *output;
    float setpoint;
    float last_input;
    float out_min, out_max;

public:
    float error_sum;

    PID(float* input, float* output, float setpoint, float Kp, float Ki, float Kd, float out_min, float out_max);

    void setTunings(float newKp, float newKi, float newKd);

    void compute(float dt); 

    void reset();
};
