#include "DisplayManager.h"

DisplayManager::DisplayManager(uint8_t width, uint8_t height, uint8_t sdaPin, uint8_t sclPin, uint8_t address)
  : _sdaPin(sdaPin), _sclPin(sclPin), _address(address), _width(width), _height(height), _display(width, height, &Wire, -1) {}

void DisplayManager::begin() {
  Wire.begin((int)_sdaPin, (int)_sclPin);
  if (!_display.begin(SSD1306_SWITCHCAPVCC, _address)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(0, 0);
  _display.display();
}

void DisplayManager::clear() {
  _display.clearDisplay();
  _display.setCursor(0, 0);
}

void DisplayManager::printLines(const String& line1, const String& line2, const String& line3, const String& line4, const String& line5, const String& line6, const String& line7) {
  clear();
  _display.setCursor(0, 0);
  _display.println(line1);
  if (line2.length()) _display.println(line2);
  if (line3.length()) _display.println(line3);
  if (line4.length()) _display.println(line4);
  if (line5.length()) _display.println(line5);
  if (line5.length()) _display.println(line6);
  if (line5.length()) _display.println(line7);
  _display.display();
}

void DisplayManager::showStartup(const String& title) {
  _display.clearDisplay();
  _display.setCursor(0, 0);
  _display.println("--==[ Booting ]==--");
  _display.println(title);
  _display.display();
}

void DisplayManager::showError(const String& message) {
  clear();
  _display.setCursor(0, 0);
  _display.setTextColor(SSD1306_WHITE);
  _display.println("!! ERROR !!");
  _display.println();
  _display.println(message);
  _display.display();
}
