#include "OpenPAPMenu.h"

// --- Submenus ---
MenuList calibrationsMenu = MenuList({
    MenuItem("Cal. Speed Control", calibrateESC),
    MenuItem("Calibrate CPAP", calibratePID),
    MenuItem("Back", goBack)
});

MenuList testComponentsMenu = MenuList({
    MenuItem("Test Motor", testMotor),
    MenuItem("Test Press. Sensor", testPressure),
    MenuItem("Back", goBack)
});

MenuList settingsMenu = MenuList({
    MenuItem("Calibrations", &calibrationsMenu),
    MenuItem("Test Components", &testComponentsMenu),
    MenuItem("Back", goBack)
});

MenuList mainMenu = MenuList({
    MenuItem("Start Therapy", beginTherapy),
    MenuItem("Settings", &settingsMenu),
    MenuItem("About", showAbout)
});

// --- Actions ---
TherapyView therapyView;
MotorTestView motorTestView;
PressureTestView pressureTestView;
ESCCalibrationView escCalibrationView;
PIDCalibrationView pidCalibrationView;
void notImplemented() {
    Serial.println("Not implemented");
}

void beginTherapy() {
    Serial.println("Entering therapy loop...");
    menu.setActiveView(&therapyView);
}

void testMotor() {
    Serial.println("Entering motor test...");
    menu.setActiveView(&motorTestView);
}

void testPressure() {
    Serial.println("Entering pressure test...");
    menu.setActiveView(&pressureTestView);
}

void calibrateESC() {
    Serial.println("Entering ESC calibration mode...");
    preferences.begin("OpenPAP", false);
    preferences.putBool("esc_cal", true);
    Serial.print("Wrote to preferences: ");
    Serial.println(preferences.getBool("esc_cal", false));
    preferences.end();
    menu.setActiveView(&escCalibrationView);
}

void calibratePID() {
    Serial.println("Entering PID calibration...");
    menu.setActiveView(&pidCalibrationView);
}

void showAbout() {
    Serial.println("OpenPAP v1.0");
    // Display info on screen
}

void goBack() {
    menu.goBack();  // Built-in function to navigate back in menu
}

// --- Active Views ---

// --- Therapy ---

void TherapyView::loop(int delta, bool buttonPressed) {
    static enum { INIT, RAMP_UP, THERAPY, RAMP_DOWN, DONE} state = INIT;
    static float input = 0.0;
    static float output = 0.0;
    static float setpoint = 12.0;
    static const float out_min = 0.0;
    static const float out_max = 1.0;
    static unsigned long last_read_time = 0;
    static unsigned long current_read_time = 0;
    static float dt = 0.001f;
    static float K = 0.0f;
    static float theta = 0.0f;
    static float tau = 0.0f;
    static const float FAULT_TOLERANCE_FACTOR = 50.0f;
    static float FAULT_THRESHOLD = 0.0f;
    static PID pid(&input, &output, setpoint, 1.0, 0.0, 0.0, out_min, out_max);
    static MeasurementBuffer<256> throttleBuffer;
    static MeasurementBuffer<256> timeBuffer;

    switch (state) {
        case INIT: {
            preferences.begin("OpenPAP", true);
            float Kp = preferences.getFloat("Kp", 1.0);
            float Ki = preferences.getFloat("Ki", 0.0);
            float Kd = preferences.getFloat("Kd", 0.0);
            K = preferences.getFloat("K", 1.0);
            tau = preferences.getFloat("tau", 1.0);
            theta = preferences.getFloat("theta", 1.0);
            preferences.end();
            pid.setTunings(Kp, Ki, Kd);
            pid.reset();
            _pressureBuffer.reset();
            FAULT_THRESHOLD = FAULT_TOLERANCE_FACTOR * setpoint * (tau + theta);
            input = pressure_sensor.getPressure();
            last_read_time = pressure_sensor.lastReadTime();
            state = RAMP_UP;
            break;
        }

        case RAMP_UP:
            // TODO: Add ramp up phase.
            state = THERAPY;
            break;

        case THERAPY:
            input = pressure_sensor.getPressure();
            _pressureBuffer.add(input);
            current_read_time = pressure_sensor.lastReadTime();
            timeBuffer.add(input);
            dt = (current_read_time - last_read_time) / 1000.0f;
            pid.compute(dt); // output should update as a side effect.
            esc.setThrottle(output);
            throttleBuffer.add(output);
            last_read_time = current_read_time;
            if (fabs(pid.errorSum) > FAULT_THRESHOLD ) {
                // Fault -- System is not responding
                Serial.println("Fault tolerance exceeded, exiting therapy loop.");
                state = RAMP_DOWN;
            }
            if (buttonPressed) {
                state = RAMP_DOWN;
            }
            break;

        case RAMP_DOWN:
            esc.stop(); // TODO: Add ramp down phase.
            state = DONE;
            break;

        case DONE:
            if (buttonPressed) {
                state = INIT;
                menu.exitActiveView();
            }
            break;
    }
}

void TherapyView::draw() {
    static Graph graph(_pressureBuffer, display, String(" --==[ OpenPAP ]==--"), (int16_t)128, (int16_t)48, (int16_t)0, (int16_t)16, 5.0f);

    graph.draw();
}

// --- Motor Test ---

void MotorTestView::loop(int delta, bool buttonPressed) {
    if (delta != 0) {
        float currThrottle = esc.getThrottle();
        float newThrottle = roundf((currThrottle + delta * .05) * 100) / 100;

        esc.setThrottle(newThrottle);
    }

    if (buttonPressed) {
        esc.stop();
        menu.exitActiveView();  // Return to menu
    }
}

void MotorTestView::draw() {
    display.printLines(
        "Motor Test",
        "Throttle: " + String(round(100*esc.getThrottle())) + "%",
        " ",
        "Rotate to adjust",
        "Press to exit"
    );
}

// --- Pressure Sensor Test ---
void PressureTestView::loop(int delta, bool buttonPressed) {
    if (buttonPressed) {
        Serial.println("Leaving Pressure Sensor Test...");
        menu.exitActiveView();  // Return to menu
    }
}

void PressureTestView::draw() {
    display.printLines(
        "Pressure Sensor Test",
        "--------------------",
        "Raw: " + String(pressure_sensor.getRawPressure()),
        "Pressure: " + String(pressure_sensor.getPressure()) + " cmH2O",
        "Tare Value: " + String(pressure_sensor.getZeroValue()),
        "Cal Factor: " + String((long)pressure_sensor.getCalibrationFactor()),
        "Press to exit"
    );
}

// --- ESC Calibration ---

ESCCalibrationView::ESCCalibrationView(){ _state = INIT; }

void ESCCalibrationView::afterBoot() {
    _state = MAXTHROTTLE;
}

void ESCCalibrationView::loop(int delta, bool buttonPressed) {
    switch (_state) {
        case INIT:
            if (buttonPressed) { // User cancels calibration
                preferences.begin("OpenPAP", false);
                preferences.remove("esc_cal");
                preferences.end();
                menu.exitActiveView();  // Return to menu
            }
            break;

        case MAXTHROTTLE:
            if (buttonPressed) {
                esc.stop();
                _state = MINTHROTTLE; // Go to next screen
            }
            break;

        case MINTHROTTLE:
            if (buttonPressed) {
                _state = DONE; // Go to next screen
            }
            break;

        case DONE:
            // User is stuck here, because they should reboot
            break;
    }
}

void ESCCalibrationView::draw() {
    switch (_state) {
        case INIT:
            display.printLines(
              "Motor Calibration",
              "-----------------",
              "Power off and on now,",
              "or press button to ",
              "cancel..."
            );
            break;

        case MAXTHROTTLE:
            display.printLines(
                "Motor Calibration",
                "-----------------",
                "Motor should emit 4 ",
                "beeps.",
                " ",
                "Press button before ",
                "they end."
            );
            break;

        case MINTHROTTLE:
            display.printLines(
                "Motor Calibration",
                "-----------------",
                "Two short beeps",
                "followed by two long",
                "beeps, indicates",
                "success.",
                "Press to continue..."
            );
            break;

        case DONE:
            display.printLines(
                "Motor Calibration",
                "-----------------",
                "Calibration complete!"
                " ",
                "Power off now.",
                "(If calibration was",
                "unsuccessful, retry)"
            );
            break;
    }
}

// --- PID Calibration ---

void PIDCalibrationView::loop(int delta, bool buttonPressed) {
    static enum { INIT, APPLY_STEP, LOGGING, ANALYSIS, DONE } state = INIT;
    static uint32_t startTime = 0;
    static const unsigned int MAX_SAMPLES = 160;
    static float pressureLog[MAX_SAMPLES];
    static uint32_t timeLog[MAX_SAMPLES];
    static size_t sampleCount = 0;

    switch (state) {
        case INIT:
            startTime = millis();
            sampleCount = 0;
            state = APPLY_STEP;
            break;

        case APPLY_STEP:
            esc.setThrottle(1.0);
            state = LOGGING;
            break;

        case LOGGING:
            if (sampleCount < MAX_SAMPLES) {
                pressureLog[sampleCount] = pressure_sensor.getPressure();
                timeLog[sampleCount] = millis() - startTime;
                sampleCount++;
            } else {
                esc.stop();
                state = ANALYSIS;
            }
            break;

        case ANALYSIS: {
            const size_t avgWindow = 10;
            const size_t confirmWindow = 3;

            // --- Step 1: Smoothed Initial and Final Pressure ---
            float sumStart = 0.0f, sumEnd = 0.0f;
            for (size_t i = 0; i < avgWindow; i++) {
                sumStart += pressureLog[i];
                sumEnd   += pressureLog[MAX_SAMPLES - 1 - i];
            }
            float P0 = sumStart / avgWindow;
            float Pfinal = sumEnd / avgWindow;
            float deltaP = Pfinal - P0;

            if (fabs(deltaP) < 0.01f) {
                Serial.println("Calibration failed: no significant pressure change");
                state = DONE;
                break;
            }

            // --- Step 2: Calculate Process Gain K ---
            float deltaInput = 0.5f; // Motor throttle step
            float K = deltaP / deltaInput;

            // --- Step 3: Find Dead Time (θ) ---
            float P5 = P0 + 0.05f * deltaP;
            size_t indexTheta = 0;
            for (size_t i = 0; i < sampleCount - confirmWindow; i++) {
                bool stable = true;
                for (size_t j = 0; j < confirmWindow; j++) {
                    if (pressureLog[i + j] < P5) {
                        stable = false;
                        break;
                    }
                }
                if (stable) {
                    indexTheta = i;
                    break;
                }
            }
            float theta = timeLog[indexTheta] / 1000.0f;

            // --- Step 4: Find Time Constant (τ) ---
            float P63 = P0 + 0.63f * deltaP;
            size_t indexTau = indexTheta;
            for (size_t i = indexTheta; i < sampleCount - confirmWindow; i++) {
                bool stable = true;
                for (size_t j = 0; j < confirmWindow; j++) {
                    if (pressureLog[i + j] < P63) {
                        stable = false;
                        break;
                    }
                }
                if (stable) {
                    indexTau = i;
                    break;
                }
            }
            float timeAt63 = timeLog[indexTau] / 1000.0f;
            float tau = timeAt63 - theta;

            if (tau <= 0 || theta <= 0 || K == 0) {
                Serial.println("Calibration failed: invalid tau or theta");
                state = DONE;
                break;
            }

            // --- Step 5: Apply IMC PID Tuning ---
            // This is a choice, lambda represents desired system response time.
            const float lambda = theta;

            float Kp = tau / (K * (lambda + theta));
            float Ki = Kp / tau;
            float Kd = Kp * theta / 2;

            // --- Step 6: Store Results ---
            preferences.begin("OpenPAP", false);
            preferences.putFloat("Kp", Kp);
            preferences.putFloat("Ki", Ki);
            preferences.putFloat("Kd", Kd);
            preferences.putFloat("K", K);
            preferences.putFloat("tau", tau);
            preferences.putFloat("theta", theta);
            preferences.end();

            Serial.println("Calibration complete:");
            Serial.printf("Kp = %.3f, Ki = %.3f, Kd = %.3f\n", Kp, Ki, Kd);
            Serial.printf("K = %.3f, τ = %.3f, θ = %.3f\n", K, tau, theta);

            state = DONE;
            break;
        }

        case DONE:
            // Wait for user to press button to return to menu
            if (buttonPressed) {
                state = INIT;
                menu.exitActiveView();
            }
            break;
    }
}

void PIDCalibrationView::draw() {
    display.printLines(
        "CPAP Calibration",
        "----------------",
        " ",
        "Throttle: " + String((int)(100*esc.getThrottle())) + "%",
        "Pressure: " + String(pressure_sensor.getLastReading())
    );
}
