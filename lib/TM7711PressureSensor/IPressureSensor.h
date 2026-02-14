#pragma once

struct IPressureSensor {
    virtual void begin() = 0;
    virtual void tare() = 0;

    virtual double getPressure() = 0;
    virtual double getLastReading() const = 0;

    virtual ~IPressureSensor() = default;
};
