#include "OpenPAPMenu.h"

// --- Submenus ---
MenuList calibrationsMenu = MenuList::withItems({
  MenuItem("Calibrate Motor", calibrateESC),
  MenuItem("Calibrate CPAP", calibratePID),
  MenuItem("Back", goBack)
});

MenuList testComponentsMenu = MenuList::withItems({
  MenuItem("Test Motor", testMotor),
  MenuItem("Test Press. Sensor", testPressure),
  MenuItem("Back", goBack)
});

MenuList settingsMenu = MenuList::withItems({
  MenuItem("Calibrations", nullptr, &calibrationsMenu),
  MenuItem("Test Components", nullptr, &testComponentsMenu),
  MenuItem("Back", goBack)
});

MenuList mainMenu = MenuList::withItems({
  MenuItem("Start Therapy", beginTherapy),
  MenuItem("Settings", nullptr, &settingsMenu),
  MenuItem("About", showAbout)
});

// --- Actions ---
void notImplemented() {
  Serial.println("Not implemented");
}

void beginTherapy() {
  Serial.println("Entering therapy loop...");
  menu.setActiveView(therapyLoop, therapyDraw);
}

void testMotor() {
  Serial.println("Entering motor test...");
  menu.setActiveView(motorTestLoop, motorTestDraw);
}

void testPressure() {
  Serial.println("Entering pressure test...");
  menu.setActiveView(pressureTestLoop, pressureTestDraw);
}

void calibrateESC() {
  Serial.println("Entering ESC calibration mode...");
  preferences.begin("OpenPAP", false);
  preferences.putBool("esc_cal", true);
  Serial.print("Wrote to preferences: ");
  Serial.println(preferences.getBool("esc_cal", false));
  preferences.end();
  menu.setActiveView(escCalibrationPreLoop, escCalibrationPreDraw);
}

void calibratePID() {
  Serial.println("Entering PID calibration...");
  menu.setActiveView(pidCalibrationLoop, pidCalibrationDraw);
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

void therapyLoop(int delta, bool buttonPressed) {
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
  static const float FAULT_TOLERANCE_FACTOR = 5.0f;
  static float FAULT_THRESHOLD = 0.0f;
  static PID pid(&input, &output, setpoint, 1.0, 0.0, 0.0, out_min, out_max);

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
      FAULT_THRESHOLD = FAULT_TOLERANCE_FACTOR * setpoint * tau;
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
      current_read_time = pressure_sensor.lastReadTime();
      dt = (current_read_time - last_read_time) / 1000.0f;
      pid.compute(dt); // output should update as a side effect.
      esc.setThrottle(output);
      last_read_time = current_read_time;
      if (fabs(pid.error_sum) > FAULT_THRESHOLD ) {
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

void therapyDraw() {
  display.printLines(
    "Throttle: " + String((int)(100*esc.getThrottle())) + "%",
    "Pressure: " + String(pressure_sensor.lastReading) + " cmH2O"
  );
}

// --- Motor Test ---

void motorTestLoop(int delta, bool buttonPressed) {
  if (delta != 0) {
    double currThrottle = esc.getThrottle();
    double newThrottle = currThrottle + delta * .05;

    // Update ESC here
    esc.setThrottle(newThrottle);
  }

  if (buttonPressed) {
    esc.stop();
    menu.exitActiveView();  // Return to menu
  }
}

void motorTestDraw() {
  display.printLines(
    "Motor Test",
    "Throttle: " + String((int)(100*esc.getThrottle())) + "%",
    " ",
    "Rotate to adjust",
    "Press to exit"
  );
}

// --- Pressure Sensor Test ---
void pressureTestLoop(int delta, bool buttonPressed) {
  if (buttonPressed) {
    menu.exitActiveView();  // Return to menu
  }
}

void pressureTestDraw() {
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
void escCalibrationPreLoop(int delta, bool buttonPressed) {
  if (buttonPressed) { // User cancels calibration
    preferences.begin("OpenPAP", false);
    preferences.remove("esc_cal");
    preferences.end();
    menu.exitActiveView();  // Return to menu
  }
}

void escCalibrationPreDraw() {
  display.printLines(
    "Motor Calibration",
    "-----------------",
    "Power off and on now,",
    "or press button to ",
    "cancel..."
  );
}

void escCalibrationMaxLoop(int delta, bool buttonPressed) {
  if (buttonPressed) {
    esc.stop();
    Serial.println("Leaving ESC calibration mode...");
    menu.setActiveView(escCalibrationMinLoop, escCalibrationMinDraw); // Go to next screen
  }
}

void escCalibrationMaxDraw() {
  display.printLines(
    "Motor Calibration",
    "-----------------",
    "Motor should emit 4 ",
    "beeps.",
    " ",
    "Press button before ",
    "they end."
  );
}

void escCalibrationMinLoop(int delta, bool buttonPressed) {
  if (buttonPressed) {
    menu.setActiveView(escCalibrationEndLoop, escCalibrationEndDraw);
  }
}

void escCalibrationMinDraw() {
  display.printLines(
    "Motor Calibration",
    "-----------------",
    "Two short beeps",
    "followed by two long",
    "beeps, indicates",
    "success.",
    "Press to continue..."
  );
}

void escCalibrationEndLoop(int delta, bool buttonPressed) {}

void escCalibrationEndDraw() {
  display.printLines(
    "Motor Calibration",
    "-----------------",
    "Calibration complete!"
    " ",
    "Power off now.",
    "(If calibration was",
    "unsuccessful, retry)"
  );
}

// --- PID Calibration ---

void pidCalibrationLoop(int delta, bool buttonPressed) {
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

      // --- Step 5: Apply Cohen-Coon PID Tuning ---
      float R = theta / tau;
      const float lambda = theta;

      // float Kp = (1.35f + R / 20.0f) * (tau / (K * theta));
      // float Ti = theta * (32.0f + 6.0f * R) / (13.0f + 8.0f * R); // sec
      // float Td = theta * (4.0f * R) / (11.0f + 2.0f * R);         // sec

      // float Ki = Kp / Ti;
      // float Kd = Kp * Td;
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

// void pidCalibrationLoop(int delta, bool buttonPressed) {
//   static enum { INIT, RUNNING, ANALYSIS, DONE } state = INIT;

//   static const float setpoint = 12.0f;          // Target pressure for autotune
//   static const float highThrottle = 1.0f;       // Relay ON throttle
//   static const float lowThrottle = 0.0f;        // Relay OFF throttle
//   static const size_t MAX_SAMPLES = 800;        // Max samples to collect
//   static const unsigned int minOscillations = 5; // Minimum oscillations before analysis

//   static float pressureLog[MAX_SAMPLES];
//   static uint32_t timeLog[MAX_SAMPLES];
//   static size_t sampleCount = 0;

//   static float lastError = 0.0f;
//   static unsigned long lastZeroCrossTime = 0;
//   static unsigned int zeroCrossCount = 0;
//   static unsigned long startTime = 0;

//   static float peakPressureMax = -1e6;
//   static float peakPressureMin = 1e6;

//   uint32_t now = millis();

//   switch (state) {
//     case INIT:
//       Serial.println("Starting Relay Autotune...");
//       sampleCount = 0;
//       zeroCrossCount = 0;
//       lastZeroCrossTime = now;
//       startTime = now;
//       lastError = setpoint - pressure_sensor.getPressure();
//       peakPressureMax = -1e6;
//       peakPressureMin = 1e6;

//       esc.setThrottle(highThrottle);
//       state = RUNNING;
//       break;

//     case RUNNING: {
//       if (sampleCount < MAX_SAMPLES) {
//         float pressure = pressure_sensor.getPressure();
//         float error = setpoint - pressure;

//         pressureLog[sampleCount] = pressure;
//         timeLog[sampleCount] = now - startTime;
//         sampleCount++;

//         // Track peaks for amplitude calculation
//         if (pressure > peakPressureMax) peakPressureMax = pressure;
//         if (pressure < peakPressureMin) peakPressureMin = pressure;

//         // Relay control: switch throttle on sign of error
//         if (error < 0) {
//           esc.setThrottle(lowThrottle);
//         } else {
//           esc.setThrottle(highThrottle);
//         }

//         // Detect zero-crossing of error
//         if ((error > 0 && lastError <= 0) || (error < 0 && lastError >= 0)) {
//           unsigned long zeroCrossTime = now;
//           unsigned long period = zeroCrossTime - lastZeroCrossTime;

//           // Ignore first zero crossing (no period yet)
//           if (zeroCrossCount > 0) {
//             // Accumulate period sum inside ANALYSIS phase
//           }
//           lastZeroCrossTime = zeroCrossTime;
//           zeroCrossCount++;

//           // Check if enough oscillations
//           if (zeroCrossCount >= minOscillations * 2) {
//             esc.stop();
//             state = ANALYSIS;
//           }
//         }

//         lastError = error;

//       } else {
//         Serial.println("Relay Autotune: Max samples reached without enough oscillations.");
//         esc.stop();
//         state = DONE;
//       }
//       break;
//     }

//     case ANALYSIS: {
//       // Calculate average period Pu (time between zero crossings)
//       if (zeroCrossCount < 2) {
//         Serial.println("Not enough zero crossings for analysis.");
//         state = DONE;
//         break;
//       }

//       // Calculate periods between zero crossings
//       unsigned long periodSum = 0;
//       for (size_t i = 1; i < zeroCrossCount; i++) {
//         periodSum += (timeLog[i] - timeLog[i - 1]);
//       }
//       float Pu = (periodSum / (zeroCrossCount - 1)) / 1000.0f; // seconds

//       // Calculate amplitude of pressure oscillations
//       float amplitude = peakPressureMax - peakPressureMin;

//       // Calculate relay output amplitude
//       float relayAmplitude = highThrottle - lowThrottle;

//       if (amplitude < 0.01f) {
//         Serial.println("Calibration failed: Oscillation amplitude too low");
//         state = DONE;
//         break;
//       }

//       // Calculate ultimate gain Ku = (4 * relayAmplitude) / (pi * amplitude)
//       float Ku = (4.0f * relayAmplitude) / (3.14159f * amplitude);

//       // Ziegler-Nichols PID tuning parameters
//       float Kp = 0.45f * Ku;
//       float Ki = Kp / (2.2 * Pu);
//       float Kd = Kp * (Pu / 6.3);

//       Serial.println("Relay Autotune complete:");
//       Serial.printf("Pu = %.3f s, Ku = %.3f\n", Pu, Ku);
//       Serial.printf("PID Gains: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", Kp, Ki, Kd);

//       // Store PID gains
//       preferences.begin("OpenPAP", false);
//       preferences.putFloat("Kp", Kp);
//       preferences.putFloat("Ki", Ki);
//       preferences.putFloat("Kd", Kd);
//       preferences.end();

//       state = DONE;
//       break;
//     }

//     case DONE:
//       if (buttonPressed) {
//         state = INIT;
//         menu.exitActiveView();
//       }
//       break;
//   }
// }


void pidCalibrationDraw() {
  display.printLines(
    "CPAP Calibration",
    "----------------",
    " ",
    "Throttle: " + String((int)(100*esc.getThrottle())) + "%",
    "Pressure: " + String(pressure_sensor.lastReading)
  );
}
