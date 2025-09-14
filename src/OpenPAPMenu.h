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
void therapyLoop(int delta, bool buttonPressed);
void therapyDraw();
void motorTestLoop(int delta, bool buttonPressed);
void motorTestDraw();
void pressureTestLoop(int delta, bool buttonPressed);
void pressureTestDraw();
void escCalibrationPreLoop(int delta, bool buttonPressed);
void escCalibrationPreDraw();
void escCalibrationMaxLoop(int delta, bool buttonPressed);
void escCalibrationMaxDraw();
void escCalibrationMinLoop(int delta, bool buttonPressed);
void escCalibrationMinDraw();
void escCalibrationEndLoop(int delta, bool buttonPressed);
void escCalibrationEndDraw();
void pidCalibrationLoop(int delta, bool buttonPressed);
void pidCalibrationDraw();

extern MenuList mainMenu;
extern MenuList settingsMenu;
extern MenuList testComponentsMenu;
extern ESCController esc;
extern DisplayManager display;
extern TM7711PressureSensor pressure_sensor;
#endif
