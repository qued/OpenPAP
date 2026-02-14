#include "PID.h"

PID::PID(float* input, float* output, float setpoint, float Kp, float Ki, float Kd, float outMin, float outMax)
    : _input(input), _output(output), _setpoint(setpoint), _Kp(Kp), _Ki(Ki), _Kd(Kd), _outMin(outMin), _outMax(outMax) {

    errorSum = 0;
    _lastInput = *input;
}

void PID::setTunings(float newKp, float newKi, float newKd) {
    _Kp = newKp;
    _Ki = newKi;
    _Kd = newKd;
}

void PID::compute(float dt) {
    if (dt == 0.0) {return;}
    float in = *_input;
    float err = _setpoint - in;
    float dInput = (in - _lastInput) / dt;

    // Anti-windup: only integrate if not saturated
    // float potentialSum = outputSum + Ki * err * dt;
    // if ((potentialSum < _outMax && potentialSum > _outMin) ||
    //     (potentialSum > _outMax && err < 0) ||
    //     (potentialSum < _outMin && err > 0)) {
    //   outputSum = potentialSum;
    // }
    errorSum += err * dt;

    float new_output = *_output + _Kp * err + _Ki * errorSum - _Kd * dInput;
    new_output = new_output < _outMax ? new_output : _outMax;
    new_output = new_output > _outMin ? new_output : _outMin;
    *_output = new_output;
    _lastInput = in;
}

void PID::reset() {
    _lastInput = *_input;
    errorSum = 0;
}
