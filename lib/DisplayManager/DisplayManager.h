#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
public:
  DisplayManager(uint8_t width = 128, uint8_t height = 64, uint8_t sdaPin=21, uint8_t sclPin=22, uint8_t address = 0x3C);

  void begin();
  void clear();
  void printLines(const String& line1, const String& line2 = "", const String& line3 = "", const String& line4 = "", const String& line5 = "", const String& line6 = "", const String& line7 = "");
  void printTextAtLoc(const String& text, int16_t x, int16_t y);
  void showStartup(const String& title);
  void showError(const String& message);
  void drawLine(uint x0, uint y0, uint x1, uint y1);
  void drawFastHLine(int16_t x, int16_t yPos, int16_t width);
  void display();

private:
  uint8_t _sdaPin, _sclPin;
  uint8_t _address;
  uint8_t _width, _height;
  Adafruit_SSD1306 _display;
};
