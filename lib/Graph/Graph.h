#pragma once

#include "MeasurementBuffer.h"
#include "DisplayManager.h"

class Graph {
public:
    Graph(const IMeasurementSource& source, DisplayManager& display, const String& header, int16_t width, int16_t height, int16_t xpos, int16_t ypos, float gridLineSpacing);

    void setHeader(const String& text);
    void setDimensions(int16_t width, int16_t height);
    void draw();
    void setPosition(int16_t x, int16_t y);
    void setGridlineSpacing(float spacing);

private:
    const IMeasurementSource& _data;
    DisplayManager& _display;
    int16_t _width, _height;
    int16_t _x = 0;
    int16_t _y = 0;
    float _gridlineSpacing = 5.0f;
    String _header;
};
