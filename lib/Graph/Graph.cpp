#include "Graph.h"

Graph::Graph(const IMeasurementSource& source, DisplayManager& display, const String& header, int16_t width, int16_t height, int16_t xpos, int16_t ypos, float gridlineSpacing)
    : _data(source), _display(display), _header(header), _width(width), _height(height), _x(xpos), _y(ypos), _gridlineSpacing(gridlineSpacing) {}

void Graph::setHeader(const String& text) {
    _header = text;
}

void Graph::setDimensions(int16_t w, int16_t h) {
    _width = w;
    _height = h;
}

void Graph::setPosition(int16_t x, int16_t y) {
    _x = x;
    _y = y;
}

void Graph::setGridlineSpacing(float spacing) {
    _gridlineSpacing = spacing;
}

void Graph::draw() {
    size_t count = _data.size();
    if (count < 2) return;
    _display.clear();

    float minVal = _data.min();
    float maxVal = _data.max();
    float range = maxVal - minVal;
    if (range == 0) range = 1.0f;  // prevent divide by zero


    // --- Draw horizontal gridlines ---
    if (_gridlineSpacing > 0.0f) {
        // Start from the first gridline at or below minVal
        int startLine = static_cast<int>(std::floor(minVal / _gridlineSpacing));
        int endLine   = static_cast<int>(std::ceil(maxVal / _gridlineSpacing));

        for (int i = startLine; i <= endLine; ++i) {
            float gridVal = i * _gridlineSpacing;
            if (gridVal < minVal || gridVal > maxVal) continue;

            int yPos = _y + _height - ((gridVal - minVal) / range) * _height;
            _display.drawFastHLine(_x+17, yPos, _width-17);

            // Set cursor for the label — adjust -6 or so for vertical centering
            _display.printTextAtLoc(String((int)gridVal), _x+2, yPos);
        }
    }

    int lastX = _x;
    int lastY = _y + _height - ((_data.get(0) - minVal) / range) * _height;

    for (size_t i = 1; i < count; ++i) {
        uint currX = round(_x + (i * _width) / (count - 1));
        uint currY = round(_y + _height - ((_data.get(i) - minVal) / range) * _height);

        _display.drawLine(lastX, lastY, currX, currY);

        lastX = currX;
        lastY = currY;
    }
    _display.printTextAtLoc(_header, 0, 0);
    _display.display();
}
