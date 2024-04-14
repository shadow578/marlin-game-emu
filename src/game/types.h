#pragma once

#include <stdint.h>
#include "renderer.h"
#include "../compat.h"

typedef struct { int8_t x, y; } pos_t;

// Simple 8:8 fixed-point
typedef int16_t fixed_t;
#define FTOF(F) fixed_t((F)*256.0f)
#define PTOF(P) (float(P)*(1.0f/256.0f))
#define BTOF(X) (fixed_t(X)<<8)
#define FTOB(X) int8_t(fixed_t(X)>>8)

class MarlinGame {
protected:
  static int score;
  static uint8_t game_state;
  static millis_t next_frame;
  static GameRenderer screen;

  static bool game_frame();
  static void draw_game_over();
  static void exit_game();
public:
  static void init_game(const uint8_t init_state, const screenFunc_t screen);
};
