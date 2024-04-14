#pragma once
#include <cstdlib>

#define GAME_FONT_WIDTH 6
#define GAME_FONT_ASCENT 8
#define GAME_WIDTH 100
#define GAME_HEIGHT 100

typedef long millis_t;
typedef void (*screenFunc_t)(void);

#define LCDVIEW_CALL_NO_REDRAW 0

#define PAGE_CONTAINS(a, b) true
#define PAGE_UNDER(a) true

#define _MIN(a, b) ((a) < (b) ? (a) : (b))
#define _MAX(a, b) ((a) > (b) ? (a) : (b))
#define WITHIN(x, a, b) ((x) >= (a) && (x) <= (b))

#define F(s) s

inline int random(int min, int max) {
  return min + rand() % (max - min);
}

class UiCompat {
public:
  void draw() { current_screen(); }

public:
  bool first_page = true;
  int encoderPosition = 0;

  void refresh(int ignored) {}
  bool get_blink() { return true; }

  void defer_status_screen() {}
  void goto_previous_screen_no_defer() {}

  void goto_screen(screenFunc_t screen) { current_screen = screen; }
  bool use_click() { return false; }

private:
  screenFunc_t current_screen = nullptr;
};

extern UiCompat ui;

