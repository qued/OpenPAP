#include "Graph.h"

Graph::Graph(const IMeasurementSource& source, DisplayManager& display, const String& header, int16_t width, int16_t height, int16_t xpos, int16_t ypos, float gridlineSpacing)
    : data(source), display(display), header(header), width(width), height(height), x(xpos), y(ypos), gridlineSpacing(gridlineSpacing) {}

void Graph::setHeader(const String& text) {
    header = text;
}

void Graph::setDimensions(int16_t w, int16_t h) {
    width = w;
    height = h;
}

void Graph::setPosition(int16_t x_, int16_t y_) {
    x = x_;
    y = y_;
}

void Graph::setGridlineSpacing(float spacing) {
    gridlineSpacing = spacing;
}

void Graph::draw() {
    size_t count = data.size();
    if (count < 2) return;
    display.clear();

    float minVal = data.min();
    float maxVal = data.max();
    float range = maxVal - minVal;
    if (range == 0) range = 1.0f;  // prevent divide by zero


    // --- Draw horizontal gridlines ---
    if (gridlineSpacing > 0.0f) {
        // Start from the first gridline at or below minVal
        int startLine = static_cast<int>(std::floor(minVal / gridlineSpacing));
        int endLine   = static_cast<int>(std::ceil(maxVal / gridlineSpacing));

        for (int i = startLine; i <= endLine; ++i) {
            float gridVal = i * gridlineSpacing;
            if (gridVal < minVal || gridVal > maxVal) continue;

            int yPos = y + height - ((gridVal - minVal) / range) * height;
            display.drawFastHLine(x+17, yPos, width-17);

            // Set cursor for the label — adjust -6 or so for vertical centering
            display.printTextAtLoc(String((int)gridVal), x+2, yPos);
        }
    }

    int lastX = x;
    int lastY = y + height - ((data.get(0) - minVal) / range) * height;

    for (size_t i = 1; i < count; ++i) {
        uint currX = round(x + (i * width) / (count - 1));
        uint currY = round(y + height - ((data.get(i) - minVal) / range) * height);

        display.drawLine(lastX, lastY, currX, currY);

        lastX = currX;
        lastY = currY;
    }
    display.printTextAtLoc(header, 0, 0);
    display.display();
}
