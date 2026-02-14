#include "TM7711PressureSensor.h"

TM7711PressureSensor::TM7711PressureSensor(uint8_t sckPin, uint8_t outPin)
    : _sckPin(sckPin), _outPin(outPin), _calibrationFactor(1.0), _zeroValue(0), _lastReading(0) {}

void TM7711PressureSensor::begin() {
    _scale.begin(_outPin, _sckPin);
}

void TM7711PressureSensor::tare() {
    _zeroValue = _scale.read_average(30);
}

double TM7711PressureSensor::getPressure() {
    float raw = getRawPressure();
    _lastReading = (double)(raw - _zeroValue) / _calibrationFactor;
    return _lastReading;
}

double TM7711PressureSensor::getLastReading() const {
  return _lastReading;
}

float TM7711PressureSensor::getRawPressure() {
    return _scale.read();
}

float TM7711PressureSensor::getZeroValue() const {
    return _zeroValue;
}

unsigned long TM7711PressureSensor::lastReadTime() {
    return _scale.last_time_read();
}

double TM7711PressureSensor::getCalibrationFactor() const {
    return _calibrationFactor;
}

void TM7711PressureSensor::setCalibrationFactor(double factor) {
    _calibrationFactor = factor;
}

void TM7711PressureSensor::setRate(TM7711PressureSensorRate rate){
    switch (rate) {
        case RATE10Hz:
            _scale.set_gain(HX711_CHANNEL_A_GAIN_128);
            break;

        case RATE40Hz:
            _scale.set_gain(HX711_CHANNEL_A_GAIN_64);
            break;
    }
}
