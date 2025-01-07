#pragma once

#include "../types.h"
#include "assert.h"

// size of the game's print bed, in cells
constexpr size_t PRINTIT_BED_WIDTH = 10;
constexpr size_t PRINTIT_BED_HEIGHT = 20;

class PrintItGame : MarlinGame
{
public:
  static void enter_game();
  static void game_screen();

private:
  struct bed_t
  {
    uint16_t bed[PRINTIT_BED_HEIGHT];

    void clear()
    {
      memset(bed, 0, sizeof(bed));
    }

    void set(const uint8_t x, const uint8_t y, const bool value)
    {
      assert(x < PRINTIT_BED_WIDTH);
      assert(y < PRINTIT_BED_HEIGHT);

      bed[y] &= ~(1 << x);
      bed[y] |= (value << x);
    }

    bool get(const uint8_t x, const uint8_t y) const
    {
      assert(x < PRINTIT_BED_WIDTH);
      assert(y < PRINTIT_BED_HEIGHT);

      return (bed[y] & (1 << x)) != 0;
    }

    /**
     * Check if the given coordinates collide with anything on the board.
     * @param x The x coordinate.
     * @param y The y coordinate.
     * @return
     * - 0 - in bounds
     * - 1 - x is out of bounds
     * - 2 - y is out of bounds
     * - 3 - collision with block
     */
    uint8_t check_collision(const uint8_t x, const uint8_t y) const
    {
      if (x >= PRINTIT_BED_WIDTH)
      {
        return 1;
      }
      if (y >= PRINTIT_BED_HEIGHT)
      {
        return 2;
      }
      if (get(x, y)) // block not empty?
      {
        return 3;
      }
      return 0;
    }
  };

  static_assert((sizeof(PrintItGame::bed_t::bed) * 8) >= PRINTIT_BED_WIDTH * PRINTIT_BED_HEIGHT, "bed_t is too small to fit requested bed size");

  struct falling_t
  {
    uint8_t x;
    uint8_t y;
    bool is_falling;
  };

public:
  struct state_t
  {
    bed_t bed;
    falling_t falling;
  };

private:
  //static void on_falling_committed(const falling_t &falling); // called when a falling block is committed to the board, for scoring

  static bool handle_player_input(const bed_t &bed, falling_t &falling); // handle player input for moving the nozzle and spawning new blocks (spawn only when no block is active)
  static bool handle_falling_gravity(const bed_t &bed, falling_t &falling); // update falling block by applying gravity, return true when landed and should commit

  static void commit_falling(const falling_t &falling, bed_t &bed); // commit the falling block to the board where it's currently at

  static void draw_bed(const uint8_t screen_x, const uint8_t screen_y, const bed_t &bed); // draw static board elements
  static void draw_falling(const falling_t &falling); // draw falling block
};

extern PrintItGame printit_game;
