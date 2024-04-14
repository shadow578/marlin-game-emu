#include "game_renderer.h"

int color = 0;

void GameRenderer::frame_start() {}

void GameRenderer::frame_end() {}

void GameRenderer::set_color(const uint8_t c) {
  color = c;
}

void GameRenderer::draw_hline(const game_dim_t x, const game_dim_t y, const game_dim_t w) {}

void GameRenderer::draw_vline(const game_dim_t x, const game_dim_t y, const game_dim_t h) {}

void GameRenderer::draw_frame(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h) {}

void GameRenderer::draw_box(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h) {}

void GameRenderer::draw_pixel(const game_dim_t x, const game_dim_t y) {}

void GameRenderer::draw_bitmapP(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h, const uint8_t* bitmap) {}

int GameRenderer::draw_string(const game_dim_t x, const game_dim_t y, const char* str) {}

void GameRenderer::draw_int(const game_dim_t x, const game_dim_t y, const int value) {}
