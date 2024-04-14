#include "game_renderer.h"
#include "compat.h"
#include "iostream"

constexpr bool debug = false;
#define I(x) static_cast<int>(x)

int color = 0;
static HDC hdc;

constexpr game_dim_t screen_to_game(const screen_dim_t x)
{
  return x / SCALE;
}

constexpr screen_dim_t game_to_screen(const game_dim_t x)
{
  return x * SCALE;
}

void set_hdc(HDC the_hdc)
{
  hdc = the_hdc;
}

void GameRenderer::frame_start()
{
  // draw full size rectangle in black
  int prev_color = color;
  color = 0;

  draw_box(0, 0, GAME_WIDTH, GAME_HEIGHT);

  color = prev_color;

  if (debug)
  {
    std::cout << "-- frame start --" << std::endl;
  }
}

void GameRenderer::frame_end()
{
  if (debug)
  {
    std::cout << "-- frame end --" << std::endl;
  }
}

void GameRenderer::set_color(const uint8_t c)
{
  color = c;
}

void GameRenderer::draw_hline(const game_dim_t x, const game_dim_t y, const game_dim_t w)
{
  if (debug)
  {
    std::cout << "hline @ " << I(x) << "," << I(y) << "; w=" << I(w) << std::endl;
  }

  draw_box(x, y, w, 1);
}

void GameRenderer::draw_vline(const game_dim_t x, const game_dim_t y, const game_dim_t h)
{
  if (debug)
  {
    std::cout << "vline @ " << I(x) << "," << I(y) << "; h=" << I(h) << std::endl;
  }

  draw_box(x, y, 1, h);
}

void GameRenderer::draw_frame(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h)
{
  if (debug)
  {
    std::cout << "frame @ " << I(x) << "," << I(y) << "; w=" << I(w) << ", h=" << I(h) << std::endl;
  }

  // draw as a series of lines
  draw_hline(x, y, w);
  draw_hline(x, y + h - 1, w);
  draw_vline(x, y, h);
  draw_vline(x + w - 1, y, h);
}

void GameRenderer::draw_box(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h)
{
  if (debug)
  {
    std::cout << "box @ " << I(x) << "," << I(y) << "; w=" << I(w) << ", h=" << I(h) << std::endl;
  }

  RECT rect = {
      .left = game_to_screen(x),
      .top = game_to_screen(y),
      .right = game_to_screen(w) + game_to_screen(x),
      .bottom = game_to_screen(h) + game_to_screen(y),
  };
  HBRUSH brush = CreateSolidBrush(color == 0 ? RGB(0, 0, 0) : RGB(255, 255, 255));
  FillRect(hdc, &rect, brush);
  DeleteObject(brush);
}

void GameRenderer::draw_pixel(const game_dim_t x, const game_dim_t y)
{
  if (debug)
  {
    std::cout << "pixel @ " << I(x) << "," << I(y) << std::endl;
  }

  draw_box(x, y, 1, 1);
}

void GameRenderer::draw_bitmapP(const game_dim_t x, const game_dim_t y, const game_dim_t bytes_per_row, const game_dim_t rows, const pgm_bitmap_t bitmap)
{
  if (debug)
  {
    std::cout << "bitmap @ " << I(x) << "," << I(y) << "; w=" << I(bytes_per_row) << ", h=" << I(rows) << std::endl;
  }

  // DWIN theorethically supports bitmaps, but most screens don't support it
  // (either because they use an older kernel version, or because they just (badly) emulate the DWIN protocol).
  // So instead, we draw the bitmap as a series of pixels, effectively emulating the draw call.
  // This will totally suck for performance, but it's the best we can do.
  for (game_dim_t row = 0; row < rows; row++)
  {
    for (game_dim_t col = 0; col < bytes_per_row; col++)
    {
      const uint8_t byte = bitmap[(row * bytes_per_row) + col];
      for (uint8_t bit = 0; bit < 8; bit++)
      {
        // assume that the screen area is cleared before drawing
        if (byte & (1 << bit))
        {
          draw_pixel(x + (col * 8) + (7 - bit + 1), y + row);
        }
      }
    }
  }
}

int GameRenderer::draw_string(const game_dim_t x, const game_dim_t y, const char *str)
{
  if (debug)
  {
    std::cout << "string @ " << I(x) << "," << I(y) << ": '" << str << "'" << std::endl;
  }

  auto re = RECT{
      .left = game_to_screen(x),
      .top = game_to_screen(y),
      .right = game_to_screen(GAME_WIDTH),
      .bottom = game_to_screen(GAME_HEIGHT),
  };

  SetTextColor(hdc, color == 0 ? RGB(255, 255, 255) : RGB(0, 0, 0));
  DrawText(hdc, str, -1, &re, DT_LEFT | DT_TOP | DT_SINGLELINE);
}

void GameRenderer::draw_int(const game_dim_t x, const game_dim_t y, const int value)
{
  if (debug)
  {
    std::cout << "int @ " << I(x) << "," << I(y) << ": " << value << std::endl;
  }

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%d", value);
  draw_string(x, y, buffer);
}
