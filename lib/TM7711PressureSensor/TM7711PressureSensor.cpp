#include "TM7711PressureSensor.h"

TM7711PressureSensor::TM7711PressureSensor(uint8_t sck_pin, uint8_t out_pin)
    : _sck_pin(sck_pin), _out_pin(out_pin), _calibration_factor(1.0), _zero_value(0) {}

void TM7711PressureSensor::begin() {
    _scale.begin(_out_pin, _sck_pin);
}

void TM7711PressureSensor::tare() {
    _zero_value = _scale.read_average(30);
}

double TM7711PressureSensor::getPressure() {
    float raw = getRawPressure();
    lastReading = (double)(raw - _zero_value) / _calibration_factor;
    return lastReading;
}

float TM7711PressureSensor::getRawPressure() {
    return _scale.read();
}

float TM7711PressureSensor::getZeroValue() {
    return _zero_value;
}

unsigned long TM7711PressureSensor::lastReadTime() {
    return _scale.last_time_read();
}

double TM7711PressureSensor::getCalibrationFactor() {
    return _calibration_factor;
}

void TM7711PressureSensor::setCalibrationFactor(double factor) {
    _calibration_factor = factor;
}

void TM7711PressureSensor::setRate(TM7711PressureSensorRate rate){
    switch (rate) {
        case RATE10Hz:
            _scale.set_gain(HX711_CHANNEL_A_GAIN_128);

        case RATE40Hz:
            _scale.set_gain(HX711_CHANNEL_A_GAIN_64);
    }
}
