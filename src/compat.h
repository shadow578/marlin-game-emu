#pragma once
#include <stdint.h>
#include <cstdlib>
#include <windows.h>
#include <math.h>

#define GAME_FONT_WIDTH 1
#define GAME_FONT_ASCENT 1
#define GAME_WIDTH 128
#define GAME_HEIGHT 64

typedef uint8_t game_dim_t;
typedef const uint8_t* pgm_bitmap_t;

#define _BUZZ(D,F)

typedef long millis_t;
typedef void (*screenFunc_t)(void);

#define LCDVIEW_CALL_NO_REDRAW 0

#define PAGE_CONTAINS(a, b) true
#define PAGE_UNDER(a) true

#define PROGMEM

#define _MIN(a, b) ((a) < (b) ? (a) : (b))
#define _MAX(a, b) ((a) > (b) ? (a) : (b))
#define WITHIN(x, a, b) ((x) >= (a) && (x) <= (b))

#define constrain(x, a, b) _MIN(_MAX(x, a), b)

typedef const char *PGM_P;
#define PSTR(s) s

#define TEST(x, b) ((x) & (1 << (b)))

#define LIMIT(x, a, b) _MIN(_MAX(x, a), b)

#define _BV(b) (1 << (b))

#define COUNT(a) (sizeof(a) / sizeof(a[0]))

#define F(s) s

inline int random(int min, int max)
{
  return min + rand() % (max - min);
}

inline millis_t millis()
{
  return GetTickCount();
}

extern void goto_menu_screen();

class UiCompat
{
public:
  void draw() { current_screen(); }

public:
  bool first_page = true;
  int encoderPosition = 0;
  bool did_click = false;

  void refresh(int ignored) {}
  bool get_blink() { return true; }

  void defer_status_screen() {}
  void goto_previous_screen_no_defer() { goto_menu_screen();  }

  void goto_screen(screenFunc_t screen)
  {
    current_screen = screen;
  }

  bool use_click()
  {
    if (did_click)
    {
      did_click = false;
      return true;
    }

    return false;
  }

  bool button_pressed() { return did_click; }

private:
  screenFunc_t current_screen = nullptr;
};

extern UiCompat ui;
