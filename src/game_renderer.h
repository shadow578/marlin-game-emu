#pragma once
#include "game/types.h"
#include <windows.h>

typedef int screen_dim_t;
constexpr int SCALE = 8;

void set_hdc(HDC hdc);

void set_color_enabled(const bool enabled);
bool get_color_enabled();
