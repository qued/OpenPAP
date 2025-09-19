#include <Arduino.h>
#include <Preferences.h>
#include "ESC.h"
#include "KY_040.h"
#include "DisplayManager.h"
#include "TM7711PressureSensor.h"
#include "MenuSystem.h"
#include "DebouncedButton.h"
#include "PID.h"
#include "OpenPAPMenu.h"

// --- Pin Definitions ---
// ESC Motor Controller
#define ESC_PIN 15
// Rotary Encoder
#define SW_PIN 5
#define CLK_PIN 18
#define DT_PIN 19
// Display
#define SDA_PIN 21
#define SCL_PIN 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Pressure Sensor
#define SCK_PIN 16
#define OUT_PIN 17

#define CALIBRATION_FACTOR 28426.0

// --- Globals ---
ESC esc(ESC_PIN);
RotaryEncoder encoder(CLK_PIN, DT_PIN);
DisplayManager display(SCREEN_WIDTH, SCREEN_HEIGHT, SDA_PIN, SCL_PIN);
TM7711PressureSensor pressure_sensor(SCK_PIN, OUT_PIN);
DebouncedButton button(SW_PIN);

MenuSystem menu(&display, &mainMenu);

Preferences preferences;

void setup() {

  preferences.begin("OpenPAP", false);
  bool escCalibrate = preferences.getBool("esc_cal", false);
  esc.begin();
  if (escCalibrate) {
    // Calibration mode is triggered when the ESC is powered on and immediately receives a full throttle signal
    esc.setThrottle(1.0);
    Serial.begin(115200);
    Serial.println("ESC Calibration mode detected!");
    encoder.begin();
    display.begin();
    preferences.remove("esc_cal");  // Make sure the next boot is in normal mode
    preferences.end();
    escCalibrationView.afterBoot();
    menu.setActiveView(&escCalibrationView);
  } else {
    float Kp = preferences.getFloat("Kp", 1.0);
    float Ki = preferences.getFloat("Ki", 0.0);
    float Kd = preferences.getFloat("Kd", 0.0);
    preferences.end();
    Serial.begin(115200);
    esc.arm();
    encoder.begin();
    display.begin();
    display.showStartup("Starting...");
  }

  pressure_sensor.begin();
  pressure_sensor.setRate(RATE40Hz);
  pressure_sensor.tare();
  pressure_sensor.setCalibrationFactor(CALIBRATION_FACTOR);

  menu.draw();
}

void loop() {
  button.update();

  menu.update(encoder.getDelta(), button.wasPressed());
}