#pragma once

struct IMotor {
    virtual void begin() = 0;
    virtual void arm() = 0;
    virtual void stop() = 0;

    virtual void setThrottle(float value) = 0;   // 0.0 → 1.0
    virtual float getThrottle() const = 0;

    virtual ~IMotor() = default;
};
