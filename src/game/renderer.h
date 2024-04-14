#pragma once

#include <stdint.h>

typedef int8_t game_dim_t;
typedef const uint8_t* pgm_bitmap_t;

/**
 * @brief
 * The GameRenderer class provides a set of functions to draw shapes on the display, for use in games.
 * Draw functions are very similar to the U8GLib functions. 
 */
class GameRenderer {
public:
  /**
   * @brief
   * Called at the start of each frame, before any draw calls. 
   */
  static void frame_start();

  /**
   * @brief
   * Called at the end of each frame, after all draw calls. 
   */
  static void frame_end();
  
  /**
   * @brief
   * The current "color" is used by all "draw" functions to set a pixel value on the display.
   * color 0 will clear the pixel, and 1 will set the pixel.
   * @see U8GLib setColorIndex
   */
  static void set_color(const uint8_t color);
  
  /**
   * @brief
   * Draw a horizontal line, starting at x/y postion (0/0 is the top left corner of the display).
   * The width of the line is w.
   * The function will draw the line in the current "color".
   * @see U8GLib drawHLine
   */
  static void draw_hline(const game_dim_t x, const game_dim_t y, const game_dim_t w);

  /**
   * @brief
   * Draw a vertical line, starting at x/y postion (0/0 is the top left corner of the display).
   * The height of the line is h.
   * The function will draw the line in the current "color".
   * @see U8GLib drawVLine
   */
  static void draw_vline(const game_dim_t x, const game_dim_t y, const game_dim_t h);

  /**
   * @brief
   * Draw a frame (outline rectangle), starting at x/y postion (0/0 is the top left corner of the display).
   * The width of the frame is w, and the height is h.
   * The function will draw the frame in the current "color".
   * @see U8GLib drawFrame
   */
  static void draw_frame(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h);

  /**
   * @brief
   * Draw a box (filled rectangle), starting at x/y postion (0/0 is the top left corner of the display).
   * The width of the box is w, and the height is h.
   * The function will draw the box in the current "color".
   * @see U8GLib drawBox
   */
  static void draw_box(const game_dim_t x, const game_dim_t y, const game_dim_t w, const game_dim_t h);

  /**
   * @brief
   * Draw a pixel at the x/y postion (0/0 is the top left corner of the display).
   * The function will draw the pixel in the current "color".
   * @see U8GLib drawPixel 
   */
  static void draw_pixel(const game_dim_t x, const game_dim_t y);

  /**
   * @brief
   * Draw a bitmap at the x/y postion (0/0 is the top left corner of the display).
   * The bitmap is specified by the array 'bitmap', which is cnt bytes long.
   * A cleared bit means: do not draw a pixel, a set bit means: draw a pixel in the current "color".
   */
  static void draw_bitmapP(const game_dim_t  x, const game_dim_t y, const game_dim_t bytes_per_row, const game_dim_t rows, const pgm_bitmap_t bitmap);

  /**
   * @brief
   * Draw a string at the x/y postion (0/0 is the top left corner of the display).
   * The function will draw the string in the current "color". 
   */
  static int draw_string(const game_dim_t x, const game_dim_t y, const char *str);

  /**
   * @brief
   * Draw an integer at the x/y postion (0/0 is the top left corner of the display).
   * The function will draw the integer in the current "color". 
   */
  static void draw_int(const game_dim_t x, const game_dim_t y, const int value);
};
