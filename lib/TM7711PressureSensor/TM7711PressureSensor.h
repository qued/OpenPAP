#pragma once

#include <Arduino.h>
#include "HX711.h"

enum TM7711PressureSensorRate {RATE10Hz, RATE40Hz};

class TM7711PressureSensor {
public:
    TM7711PressureSensor(uint8_t sckPin, uint8_t outPin);

    void begin();
    double getPressure();                     // Get calibrated pressure
    float getRawPressure();                    // Get raw reading from sensor
    double getCalibrationFactor() const;            // Get calibration factor
    float getZeroValue() const;                      // Get zero value
    double getLastReading() const;
    void setCalibrationFactor(double factor); // Set the calibration factor
    void tare();                              // Tare (zero) the sensor to the current reading
    unsigned long lastReadTime();                 // millis of last reading
    void setRate(TM7711PressureSensorRate rate);

private:
    const uint8_t _sckPin;
    const uint8_t _outPin;
    double _calibrationFactor;
    double _lastReading;
    float _zeroValue;
    HX711 _scale;
};
