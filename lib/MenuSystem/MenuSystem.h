#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include "DisplayManager.h"

class MenuList;

// Basic MenuItem structure
struct MenuItem {
  const char* label;
  void (*onSelect)();         // Callback for non-submenu items
  MenuList* submenu = nullptr;
  MenuItem() : label(nullptr), onSelect(nullptr), submenu(nullptr) {}

  MenuItem(const char* lbl, void (*cb)()) 
    : label(lbl), onSelect(cb), submenu(nullptr) {}

  MenuItem(const char* lbl, MenuList* sub)
    : label(lbl), onSelect(nullptr), submenu(sub) {}
};

class MenuList {
public:
  static const size_t MaxItems = 10;
  MenuList(std::initializer_list<MenuItem> items);
  void add(const MenuItem& item);
  const MenuItem& operator[](size_t i) const;
  size_t size() const;

private:
  MenuItem _items[MaxItems];
  size_t _size;
};

class MenuSystem {
public:
  MenuSystem(DisplayManager* display, const MenuList* root);

  // Standard menu navigation
  void navigate(int delta);
  void select();
  void goBack();

  // Modal view (custom UI screen)
  void update(int delta, bool buttonPressed);
  void setActiveView(void (*loopFunc)(int, bool), void (*drawFunc)());
  void exitActiveView();

  void draw();  // Can be called manually to refresh

private:
  DisplayManager* _display;

  // Menu state
  static const int MAX_DEPTH = 5;
  const MenuList* _menuStack[MAX_DEPTH];
  int _indexStack[MAX_DEPTH];
  int _depth;

  const MenuList* _currentMenu;
  int _currentIndex;

  // Modal state
  bool _inViewMode = false;
  void (*_activeViewLoop)(int, bool) = nullptr;
  void (*_activeViewDraw)() = nullptr;
};

#endif
