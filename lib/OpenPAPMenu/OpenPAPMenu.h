#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include <algorithm>

#include "MenuSystem.h"
#include "ESC.h"
#include "DisplayManager.h"
#include "TM7711PressureSensor.h"
#include "PID.h"
#include "MeasurementBuffer.h"
#include "Graph.h"

extern MenuSystem menu;
extern Preferences preferences;

// --- Callbacks ---
void beginTherapy();
void testMotor();
void testPressure();
void testAll();
void calibrateESC();
void calibratePID();
void calibrateSystemResponse();
void showAbout();
void goBack();
void notImplemented();

class TherapyView : public ActiveView {
public:
    void loop(int delta, bool buttonPressed) override;
    void draw() override;
private:
    MeasurementBuffer<256> _pressureBuffer;
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

class AllTestView : public ActiveView {
public:
    void loop(int delta, bool buttonPressed) override;
    void draw() override;
};

class ESCCalibrationView : public ActiveView {
public:
    ESCCalibrationView();
    void loop(int delta, bool buttonPressed) override;
    void draw() override;
    void afterBoot();
private:
    enum class State {INIT, MAXTHROTTLE, MINTHROTTLE, DONE};
    State _state;
};

class PIDCalibrationView : public ActiveView {
public:
    void loop(int delta, bool buttonPressed) override;
    void draw() override;
};

class SystemResponseCalibrationView : public ActiveView {
public:
    SystemResponseCalibrationView();
    void loop(int delta, bool buttonPressed) override;
    void draw() override;
private:
    enum class State {
        INIT,
        BEFORE_PHASE,
        SETTLING_PHASE,
        RECORDING_PHASE,
        FINAL,
        DONE
    };
    State _state;
    unsigned int _throttle;
    unsigned int _phaseStartTime;
    const unsigned int _settlingTime;
    const unsigned int _phaseTime;
};

// Helper function
float calculateMedian(std::vector<float>& readings);

extern ESCCalibrationView escCalibrationView;
extern MenuList mainMenu;
extern MenuList settingsMenu;
extern MenuList testComponentsMenu;
extern ESC esc;
extern DisplayManager display;
extern TM7711PressureSensor pressure_sensor;
