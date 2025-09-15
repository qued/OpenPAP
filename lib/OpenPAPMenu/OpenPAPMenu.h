#ifndef OPENPAP_MENU_H
#define OPENPAP_MENU_H

#include <Arduino.h>
#include <Preferences.h>

#include "MenuSystem.h"
#include "ESCController.h"
#include "DisplayManager.h"
#include "TM7711PressureSensor.h"
#include "PID.h"

extern MenuSystem menu;
extern Preferences preferences;

// --- Callbacks ---
void beginTherapy();
void testMotor();
void testPressure();
void calibrateESC();
void calibratePID();
void showAbout();
void goBack();
void notImplemented();
class TherapyView : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class MotorTestView : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class PressureTestView : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class ESCCalibrationView1 : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class ESCCalibrationView2 : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class ESCCalibrationView3 : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class ESCCalibrationView4 : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};
class PIDCalibrationView : public ActiveView {
public:
  void loop(int delta, bool buttonPressed) override;
  void draw() override;
};

extern ESCCalibrationView2 escCalibrationView2;
extern MenuList mainMenu;
extern MenuList settingsMenu;
extern MenuList testComponentsMenu;
extern ESCController esc;
extern DisplayManager display;
extern TM7711PressureSensor pressure_sensor;
#endif
