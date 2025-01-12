#pragma once

#include "../types.h"
#include "assert.h"

class MazeGame : MarlinGame
{
public:
  static void enter_game();
  static void game_screen();

private:
  struct vec2d_t
  {
    fixed_t x;
    fixed_t y;

    vec2d_t operator+(const vec2d_t &other) const
    {
      return vec2d_t::from(x + other.x, y + other.y);
    }

    vec2d_t operator*(const fixed_t scalar) const
    {
      return vec2d_t::from(
        FTOF(PTOF(x) * PTOF(scalar)), 
        FTOF(PTOF(y) * PTOF(scalar))
      );
    }

    static vec2d_t from(const fixed_t x, const fixed_t y)
    {
      return { x, y };
    }
  };

  struct world_t
  {
    const uint8_t *data;
    const uint8_t width; // must be multiple of 8
    const uint8_t height;

    bool is_in_bounds(const uint8_t x, const uint8_t y) const
    {
      return x < width && y < height;
    }

    bool get(const uint8_t x, const uint8_t y) const
    {
      assert(is_in_bounds(x, y));

      const uint8_t stride = width / 8;

      const uint8_t i = (y * stride) + (x / 8);
      const uint8_t bit = x % 8;

      return (data[i] & (1 << (7 - bit))) != 0;
    }
  };

  struct player_t
  {
    vec2d_t pos;
    fixed_t rotation; // rotation in degrees
  };

public:
  struct state_t
  {
    uint8_t world;
    player_t player;
  };

private:

  static void draw_world(const world_t *world, const player_t &player);


  const static world_t WORLDS[1];
  inline static const world_t* get_world();
};

extern MazeGame maze_game;
