#pragma once


template <size_t N>
MeasurementBuffer<N>::MeasurementBuffer()
    : _writeIndex(0), _filledSize(0) {}

template <size_t N>
void MeasurementBuffer<N>::add(float value) {
    _buffer[_writeIndex] = value;
    _writeIndex = (_writeIndex + 1) % N;
    if (_filledSize < N) {
        _filledSize++;
    }
}

template <size_t N>
float MeasurementBuffer<N>::get(size_t index) const {
    if (index >= _filledSize) return 0.0f;
    size_t actualIndex = (_writeIndex + N - _filledSize + index) % N;
    return _buffer[actualIndex];
}

template <size_t N>
void MeasurementBuffer<N>::reset() {
    _filledSize = 0;
    _writeIndex = 0;
}

template <size_t N>
size_t MeasurementBuffer<N>::size() const {
    return _filledSize;
}

template <size_t N>
size_t MeasurementBuffer<N>::capacity() const {
    return N;
}

template <size_t N>
float MeasurementBuffer<N>::max() const {
    if (_filledSize == 0) return 0.0f;
    float m = get(0);
    for (size_t i = 1; i < _filledSize; ++i) {
        float v = get(i);
        if (v > m) m = v;
    }
    return m;
}

template <size_t N>
float MeasurementBuffer<N>::min() const {
    if (_filledSize == 0) return 0.0f;
    float m = get(0);
    for (size_t i = 1; i < _filledSize; ++i) {
        float v = get(i);
        if (v < m) m = v;
    }
    return m;
}

template <size_t N>
float MeasurementBuffer<N>::average() const {
    if (_filledSize == 0) return 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < _filledSize; ++i) {
        sum += get(i);
    }
    return sum / _filledSize;
}

template <size_t N>
const float* MeasurementBuffer<N>::raw() const {
    return _buffer; // raw circular buffer, not logically ordered
}

template <size_t N>
template <typename Func>
void MeasurementBuffer<N>::forEach(Func func) const {
    for (size_t i = 0; i < _filledSize; ++i) {
        size_t idx = (_writeIndex + N - _filledSize + i) % N;
        func(_buffer[idx]);
    }
}
