#include "MenuSystem.h"

MenuList::MenuList(std::initializer_list<MenuItem> items) {
  _size = 0;
  for (const auto& item : items) {
    add(item);
  }
}

void MenuList::add(const MenuItem& item) {
  if (_size < MaxItems) {
    _items[_size++] = item;
  } else {
    Serial.println("MenuList: Too many items!");
  }
}

const MenuItem& MenuList::operator[](size_t i) const {
  static MenuItem invalidItem;
  if (i >= _size) return invalidItem;
  return _items[i];
}

size_t MenuList::size() const {
  return _size;
}

MenuSystem::MenuSystem(DisplayManager* display, const MenuList* root)
  : _display(display), _currentMenu(root), _currentIndex(0), _depth(0) {}

void MenuSystem::draw() {
  if (!_currentMenu) return;

  if (_inViewMode) {
    if (_activeViewDraw) _activeViewDraw();
    return;
  }

  String lines[5] = {"", "", "", "", ""};  // Up to 5 menu items
  for (int i = 0; i < 5 && (*_currentMenu)[i].label != nullptr; ++i) {
    lines[i] = (i == _currentIndex ? "> " : "  ");
    lines[i] += (*_currentMenu)[i].label;
  }

  _display->printLines(lines[0], lines[1], lines[2], lines[3], lines[4]);
}

void MenuSystem::navigate(int delta) {
  if (!_currentMenu) return;
  if (_inViewMode) return;

  int maxIndex = _currentMenu->size();
  if (maxIndex == 0) return;
  // while (_currentMenu[maxIndex].label != nullptr) maxIndex++;

  _currentIndex += delta;
  if (_currentIndex < 0) _currentIndex = maxIndex - 1;
  if (_currentIndex >= maxIndex) _currentIndex = 0;

  draw();
}

void MenuSystem::select() {
  if (!_currentMenu) return;
  if (_inViewMode) return;

  const MenuItem* item = &((*_currentMenu)[_currentIndex]);

  if (item->submenu) {
    if (_depth < MAX_DEPTH) {
      _menuStack[_depth] = _currentMenu;
      _indexStack[_depth] = _currentIndex;
      _depth++;

      _currentMenu = item->submenu;
      _currentIndex = 0;
      draw();
    }
  } else if (item->onSelect) {
    item->onSelect();
    draw();
  }
}

void MenuSystem::goBack() {
  if (_depth > 0) {
    _depth--;
    _currentMenu = _menuStack[_depth];
    _currentIndex = _indexStack[_depth];
    draw();
  }
}

void MenuSystem::setActiveView(void (*loopFunc)(int, bool), void (*drawFunc)()) {
  _inViewMode = true;
  _activeViewLoop = loopFunc;
  _activeViewDraw = drawFunc;
  draw();
}

void MenuSystem::exitActiveView() {
  _inViewMode = false;
  _activeViewLoop = nullptr;
  _activeViewDraw = nullptr;
  draw();
}

void MenuSystem::update(int delta, bool buttonPressed) {
  if (_inViewMode) {
    if (_activeViewLoop) _activeViewLoop(delta, buttonPressed);
    if (_activeViewDraw) _activeViewDraw();
    return;
  }

  if (delta != 0) navigate(delta);
  if (buttonPressed) select();
}
