#pragma once

#include <stddef.h>

class IMeasurementSource {
public:
    virtual size_t size() const = 0;
    virtual float get(size_t index) const = 0;
    virtual void reset() = 0;
    virtual float min() const = 0;
    virtual float max() const = 0;
    virtual ~IMeasurementSource() = default;
};

template <size_t N>
class MeasurementBuffer : public IMeasurementSource  {
public:
    MeasurementBuffer();

    void add(float value);
    float get(size_t index) const; // 0 = oldest, size()-1 = newest
    void reset();
    template <typename Func>
    void forEach(Func func) const;

    size_t size() const;
    size_t capacity() const;

    float max() const;
    float min() const;
    float average() const;

    const float* raw() const;

private:
    float _buffer[N];
    size_t _writeIndex;
    size_t _filledSize;
};

#include "MeasurementBuffer.tpp"
