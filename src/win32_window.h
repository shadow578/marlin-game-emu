#pragma once
#include <windows.h>
#include "compat.h"
#include "game_renderer.h"

namespace win32_window {
  constexpr int WINDOW_WIDTH = (GAME_WIDTH + 2) * SCALE;
  constexpr int WINDOW_HEIGHT = (GAME_HEIGHT + 6) * SCALE;

  typedef void (*draw_fn)(HDC hdc);
  typedef void (*key_fn)(WPARAM key);

  void set_draw_fn(draw_fn fn);
  void set_key_fn(key_fn fn);

  void set_target_fps(int fps);

  bool create_and_run(HINSTANCE hInstance, int nCmdShow);
} // namespace win32_window
