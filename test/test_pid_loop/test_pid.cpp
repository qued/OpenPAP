// test/test_pid_pressure_control/test_pid_pressure_control.cpp
#include <unity.h>
#include <fstream>
#include <iostream>

#include "PID.h"

float P[11];

void test_pid_increases_throttle_when_pressure_low() {
    float input = 0;
    float output = 0;

    PID controller(
        &input,
        &output,
        /* targetPressure */ 10.0f,
        /* Kp */ 0.2f,
        /* Ki */ 0.0f,
        /* Kd */ 0.0f,
        0.0f,
        1.0f
    );

    // Pressure starts low
    // sensor.setPressure(5.0);
    // motor.setThrottle(0.1f);
    output = 0.1f;
    input = 5.0f;

    controller.compute(0.100); // 100 ms timestep

    TEST_ASSERT_TRUE_MESSAGE(
        output > 0.1f,
        "Expected output > 0.1"
    );
}

void test_pid_reduces_throttle_when_pressure_high() {
    float input = 0;
    float output = 0;

    PID controller(
        &input,
        &output,
        /* targetPressure */ 10.0f,
        /* Kp */ 0.2f,
        /* Ki */ 0.0f,
        /* Kd */ 0.0f,
        0.0f,
        1.0f
    );

    // Pressure starts high
    output = 0.5f;
    input = 15.0f;

    controller.compute(0.100); // 100 ms timestep

    TEST_ASSERT_TRUE_MESSAGE(
        output < 0.5f,
        "Expected output < 0.5"
    );
}

float expected_output_from_input(float value_in) {

    int index = (int)floor(value_in * 10.0f);
    return P[index] + (P[index+1] - P[index]) * (value_in - ((float)index / 10.0f)) / (0.1f);
}

void test_pid_converges_to_desired_value() {
    P[0] = 0.0;
    P[1] = 0.0;
    P[2] = 1.5;
    P[3] = 2.5;
    P[4] = 6.0;
    P[5] = 8.0;
    P[6] = 10.0;
    P[7] = 12.0;
    P[8] = 15.0;
    P[9] = 18.0;
    P[10] = 18.5;
    float output = 0.5f;
    float input = P[5];

    PID controller(
        &input,
        &output,
        /* targetPressure */ 10.0f,
        /* Kp */ 0.002f,
        /* Ki */ 0.00000f,
        /* Kd */ 0.0000000f,
        0.0f,
        1.0f
    );

    float pressure_tail[100];
    float throttle_tail[100];
    std::ofstream file("output.csv");

    // Write header
    file << "time,input,output\n";

    int i = 0;
    for (float t=0; t<5; t+=0.01) {
        controller.compute(0.01);

        input = expected_output_from_input(output);
        pressure_tail[i % 100] = input;
        throttle_tail[i % 100] = output;
        i += 1;

        // Write data rows
        file << t << "," << input << "," << output << "\n";
    }
    file.close();

    float max_of_pressure_tail = 0.0f;
    float min_of_pressure_tail = P[10];
    float max_of_throttle_tail = 0.0;
    float min_of_throttle_tail = 1.0;

    for (i = 0; i < 100; i++) {
        if (pressure_tail[i] > max_of_pressure_tail) {
            max_of_pressure_tail = pressure_tail[i];
        }
        if (pressure_tail[i] < min_of_pressure_tail) {
            min_of_pressure_tail = pressure_tail[i];
        }
        if (throttle_tail[i] > max_of_throttle_tail) {
            max_of_throttle_tail = throttle_tail[i];
        }
        if (throttle_tail[i] < min_of_throttle_tail) {
            min_of_throttle_tail = throttle_tail[i];
        }
    }

    TEST_ASSERT_FLOAT_WITHIN(
        0.15f,
        0.6f,
        max_of_throttle_tail
    );
    TEST_ASSERT_FLOAT_WITHIN(
        0.15f,
        0.6f,
        min_of_throttle_tail
    );
    TEST_ASSERT_FLOAT_WITHIN(
        0.3f,
        10.0f,
        min_of_pressure_tail
    );
    TEST_ASSERT_FLOAT_WITHIN(
        0.3f,
        10.0f,
        max_of_pressure_tail
    );
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_pid_increases_throttle_when_pressure_low);
    RUN_TEST(test_pid_reduces_throttle_when_pressure_high);
    RUN_TEST(test_pid_converges_to_desired_value);
    return UNITY_END();
}
