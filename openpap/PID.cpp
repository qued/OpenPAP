#include "PID.h"

PID::PID(float* input, float* output, float setpoint,
      float Kp, float Ki, float Kd,
      float out_min, float out_max)
      : input(input), output(output), setpoint(setpoint),
        Kp(Kp), Ki(Ki), Kd(Kd),
        out_min(out_min), out_max(out_max) {
    
  error_sum = 0;
  last_input = *input;
}

void PID::setTunings(float newKp, float newKi, float newKd) {
  Kp = newKp;
  Ki = newKi;
  Kd = newKd;
}

void PID::compute(float dt) {
  if (dt == 0.0) {return;}
  float in = *input;
  float err = setpoint - in;
  float dInput = (in - last_input) / dt;

  // Anti-windup: only integrate if not saturated
  // float potentialSum = output_sum + Ki * err * dt;
  // if ((potentialSum < out_max && potentialSum > out_min) ||
  //     (potentialSum > out_max && err < 0) ||
  //     (potentialSum < out_min && err > 0)) {
  //   output_sum = potentialSum;
  // }
  error_sum += err * dt;

  // float out = Kp * err + output_sum - Kd * dInput;
  float out = Kp * err + Ki * error_sum - Kd * dInput;
  out = constrain(out, out_min, out_max);
  *output = out;
  last_input = in;
}

void PID::reset() {
  // output_sum = *output;
  last_input = *input;
  // output_sum = constrain(output_sum, out_min, out_max);
  error_sum = 0;
}
