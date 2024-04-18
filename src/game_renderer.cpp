#include "game_renderer.h"
#include "compat.h"
#include "iostream"

constexpr bool debug = false;
#define I(x) static_cast<int>(x)

bool color_enable = true;
COLORREF current_color = RGB(0, 0, 0);
static HDC hdc;

constexpr bool dont_count_subcalls = true;
typedef struct {
  int set_color,
  int draw_hline,
  int draw_vline,
  int draw_frame,
  int draw_box,
  int draw_pixel,
  int draw_bitmap,
  int draw_string,
  int draw_int,
} draw_call_cnt_t;

static draw_call_cnt_t draw_call_cnt = { 0 }; // number of draw calls between frame_start and frame_end
static int frame_draw_time = 0; // time between frame_start and frame_end
static int frame_wait_time = 0; // time between frame_end and frame_start

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

void set_color_enabled(const bool enabled)
{
  color_enable = enabled;
}

bool get_color_enabled()
{
  return color_enable;
}

void MarlinGame::frame_start()
{
  draw_call_cnt = { 0 };
  frame_draw_time = GetTickCount();
  frame_wait_time = frame_draw_time - frame_wait_time;
  
  // draw full size rectangle in black
  const auto prev_color = current_color;
  current_color = RGB(0, 0, 0);

  draw_box(0, 0, GAME_WIDTH, GAME_HEIGHT);

  current_color = prev_color;

  if (debug)
  {
    std::cout << "-- frame start --" << std::endl;
  }
}

void MarlinGame::frame_end()
{
  int fwt = frame_wait_time;
  frame_wait_time = GetTickCount();
  frame_draw_time = frame_wait_time - frame_draw_time;

  int total_draw_calls = draw_call_cnt.set_color 
                       + draw_call_cnt.draw_hline 
                       + draw_call_cnt.draw_vline 
                       + draw_call_cnt.draw_frame 
                       + draw_call_cnt.draw_box 
                       + draw_call_cnt.draw_pixel 
                       + draw_call_cnt.draw_bitmap 
                       + draw_call_cnt.draw_string
                       + draw_call_cnt.draw_int;

  std::cout << "draw: " << frame_draw_time << " ms (wait: " << fwt << " ms);  "
            << draw_call_cnt.set_color << "x set_color; "
            << draw_call_cnt.draw_hline << "x hline; "
            << draw_call_cnt.draw_vline << "x vline; "
            << draw_call_cnt.draw_frame << "x frame; "
            << draw_call_cnt.draw_box << "x box; "
            << draw_call_cnt.draw_pixel << "x pixel; "
            << draw_call_cnt.draw_bitmap << "x bitmap; "
            << draw_call_cnt.draw_string << "x string; "
            << draw_call_cnt.draw_int << "x int; "
            << total_draw_calls << "x calls total" 
            << std::endl;
  
  if (debug)
  {
    std::cout << "-- frame end --" << std::endl;
  }
}

void MarlinGame::set_color(const color c)
{
  if (color_enable)
  {
    switch(c)
    {
    case color::BLACK:
      current_color = RGB(0,0,0);
      break;
    case color::WHITE:
    default:
      current_color = RGB(0xff, 0xff, 0xff);
      break;

    // https://rgbcolorpicker.com/565/table
    case color::RED:
      current_color = RGB(0xFF, 0x00, 0x00);
      break;
    case color::GREEN:
      current_color = RGB(0x00, 0xFF, 0x00);
      break;
    case color::BLUE:
      current_color = RGB(0x00, 0x00, 0xFF);
      break;
    case color::YELLOW:
      current_color = RGB(0xFF, 0xFF, 0x00);
      break;
    case color::CYAN:
      current_color = RGB(0x00, 0xFF, 0xFF);
      break;
    case color::MAGENTA:
      current_color = RGB(0xFF, 0x00, 0xFF);
      break;
    }
  } else {
    switch(c)
    {
    case color::BLACK:
      current_color = RGB(0,0,0);
      break;
    case color::WHITE:
    default:
      current_color = RGB(0xff, 0xff, 0xff);
      break;
    }
  }

  draw_call_cnt.set_color++;
}

void MarlinGame::draw_hline(const game_dim_t x, const game_dim_t y, const game_dim_t w)
{
  if (debug)
  {
    std::cout << "hline @ " << I(x) << "," << I(y) << "; w=" << I(w) << std::endl;
  }

  draw_box(x, y, w, 1);

  draw_call_cnt.draw_hline++;

  if (dont_count_subcalls)
  {
    draw_call_cnt.draw_box--;
  }
}

void MarlinGame::draw_vline(const game_dim_t x, const game_dim_t y, const game_dim_t h)
{
  if (debug)
  {
    std::cout << "vline @ " << I(x) << "," << I(y) << "; h=" << I(h) << std::endl;
  }

  draw_box(x, y, 1, h);

  draw_call_cnt.draw_vline++;

  if (dont_count_subcalls)
  {
    draw_call_cnt.draw_box--;
  }
}

void MarlinGame::draw_frame(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h)
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

  draw_call_cnt.draw_frame++;

  if (dont_count_subcalls)
  {
    draw_call_cnt.draw_hline -= 2;
    draw_call_cnt.draw_vline -= 2;
  }
}

void MarlinGame::draw_box(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h)
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
  HBRUSH brush = CreateSolidBrush(current_color);
  FillRect(hdc, &rect, brush);
  DeleteObject(brush);

  draw_call_cnt.draw_box++;
}

void MarlinGame::draw_pixel(const game_dim_t x, const game_dim_t y)
{
  if (debug)
  {
    std::cout << "pixel @ " << I(x) << "," << I(y) << std::endl;
  }

  draw_box(x, y, 1, 1);

  draw_call_cnt.draw_pixel++;

  if (dont_count_subcalls)
  {
    draw_call_cnt.draw_box--;
  }
}

void MarlinGame::draw_bitmap(const game_dim_t x, const game_dim_t y, const game_dim_t bytes_per_row, const game_dim_t rows, const pgm_bitmap_t bitmap)
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

          if (dont_count_subcalls)
          {
            draw_call_cnt.draw_pixel--;
          }
        }
      }
    }
  }

  draw_call_cnt.draw_bitmap++;
}

int MarlinGame::draw_string(const game_dim_t x, const game_dim_t y, const char *str)
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

  SetTextColor(hdc, current_color);
  SetBkColor(hdc, RGB(0, 0, 0));
  DrawText(hdc, str, -1, &re, DT_LEFT | DT_TOP | DT_SINGLELINE);

  draw_call_cnt.draw_string++;
}

void MarlinGame::draw_int(const game_dim_t x, const game_dim_t y, const int value)
{
  if (debug)
  {
    std::cout << "int @ " << I(x) << "," << I(y) << ": " << value << std::endl;
  }

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%d", value);
  draw_string(x, y, buffer);

  draw_call_cnt.draw_int++;

  if (dont_count_subcalls)
  {
    draw_call_cnt.draw_string--;
  }
}
