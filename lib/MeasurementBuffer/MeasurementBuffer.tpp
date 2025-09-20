#pragma once


template <size_t N>
MeasurementBuffer<N>::MeasurementBuffer()
    : writeIndex(0), filledSize(0) {}

template <size_t N>
void MeasurementBuffer<N>::add(float value) {
    buffer[writeIndex] = value;
    writeIndex = (writeIndex + 1) % N;
    if (filledSize < N) {
        filledSize++;
    }
}

template <size_t N>
float MeasurementBuffer<N>::get(size_t index) const {
    if (index >= filledSize) return 0.0f;
    size_t actualIndex = (writeIndex + N - filledSize + index) % N;
    return buffer[actualIndex];
}

template <size_t N>
void MeasurementBuffer<N>::reset() {
    filledSize = 0;
    writeIndex = 0;
}

template <size_t N>
size_t MeasurementBuffer<N>::size() const {
    return filledSize;
}

template <size_t N>
size_t MeasurementBuffer<N>::capacity() const {
    return N;
}

template <size_t N>
float MeasurementBuffer<N>::max() const {
    if (filledSize == 0) return 0.0f;
    float m = get(0);
    for (size_t i = 1; i < filledSize; ++i) {
        float v = get(i);
        if (v > m) m = v;
    }
    return m;
}

template <size_t N>
float MeasurementBuffer<N>::min() const {
    if (filledSize == 0) return 0.0f;
    float m = get(0);
    for (size_t i = 1; i < filledSize; ++i) {
        float v = get(i);
        if (v < m) m = v;
    }
    return m;
}

template <size_t N>
float MeasurementBuffer<N>::average() const {
    if (filledSize == 0) return 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < filledSize; ++i) {
        sum += get(i);
    }
    return sum / filledSize;
}

template <size_t N>
const float* MeasurementBuffer<N>::raw() const {
    return buffer; // raw circular buffer, not logically ordered
}

template <size_t N>
template <typename Func>
void MeasurementBuffer<N>::forEach(Func func) const {
    for (size_t i = 0; i < filledSize; ++i) {
        size_t idx = (writeIndex + N - filledSize + i) % N;
        func(buffer[idx]);
    }
}
