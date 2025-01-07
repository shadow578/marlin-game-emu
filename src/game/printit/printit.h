#pragma once

#include "../types.h"
#include "assert.h"

// size of the game's print bed, in cells
constexpr size_t PRINTIT_BED_WIDTH = 16;
constexpr size_t PRINTIT_BED_HEIGHT = 20;

constexpr size_t PRINTIT_LEVEL_COUNT = 1;

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

    void set(const uint8_t x, const uint8_t y, const bool value = true)
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

    uint8_t get_set_blocks() const
    {
      uint8_t count = 0;
      for (uint8_t y = 0; y < PRINTIT_BED_HEIGHT; y++)
      {
        for (uint8_t x = 0; x < PRINTIT_BED_WIDTH; x++)
        {
          if (get(x, y))
          {
            count++;
          }
        }
      }

      return count;
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
    uint8_t level;
  };

private:
  static void on_falling_committed(const falling_t &falling);
  static void on_level_completed();

  static bool handle_player_input(const bed_t &bed, falling_t &falling);
  static bool handle_falling_gravity(const bed_t &bed, falling_t &falling);

  static void commit_falling(const falling_t &falling, bed_t &bed);

  /**
   * @return level status 
   * 0 = some blocks missing from target
   * 1 = all blocks present, level complete
   * 2 = misplaced block, level failed
   */
  static uint8_t get_level_status(const bed_t &bed);

  static void draw_bed(const uint8_t screen_x, const uint8_t screen_y, const bed_t &bed);
  static void draw_falling(const falling_t &falling);

  struct level_t
  {
    void (*init)(bed_t &bed);
  };

  static const level_t levels[PRINTIT_LEVEL_COUNT];
  static bed_t target_bed;
};

extern PrintItGame printit_game;
