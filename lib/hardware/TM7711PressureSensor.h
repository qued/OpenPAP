#pragma once

#include <Arduino.h>
#include "IPressureSensor.h"
#include "HX711.h"

enum TM7711PressureSensorRate {RATE10Hz, RATE40Hz};

class TM7711PressureSensor : public IPressureSensor {
public:
    TM7711PressureSensor(uint8_t sckPin, uint8_t outPin);

    void begin() override;
    void tare() override;

    double getPressure() override;
    double getLastReading() const override;

    // Hardware / calibration–specific API
    float getRawPressure();
    float getZeroValue() const;

    unsigned long lastReadTime();

    double getCalibrationFactor() const;
    void setCalibrationFactor(double factor);

    void setRate(TM7711PressureSensorRate rate);

private:
    const uint8_t _sckPin;
    const uint8_t _outPin;

    HX711 _scale;

    double _calibrationFactor;
    float _zeroValue;
    double _lastReading;
};
