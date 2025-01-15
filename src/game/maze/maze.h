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
    float x;
    float y;

    vec2d_t operator+(const vec2d_t &other) const
    {
      return vec2d_t::from(x + other.x, y + other.y);
    }

    vec2d_t operator-(const vec2d_t &other) const
    {
      return vec2d_t::from(x - other.x, y - other.y);
    }

    vec2d_t operator*(const float scalar) const
    {
      return vec2d_t::from(
        x * scalar,
        y * scalar
      );
    }

    float magnitude_squared() const
    {
      return (x * x) + (y * y);
    }

    float magnitude() const
    {
      return sqrtf(magnitude_squared());
    }

    static vec2d_t from(const float x, const float y)
    {
      return { x, y };
    }
  };

  struct player_t
  {
    vec2d_t pos;
    float rotation; // rotation in radians
  };

  struct entity_sprite_t
  {
    const uint8_t *sprite;
    const uint8_t width;
    const uint8_t height;

    bool get(const uint8_t u, const uint8_t v) const
    {
      assert(u < width && v < height);

      const uint8_t stride = width / 8;

      const uint8_t i = (v * stride) + (u / 8);
      const uint8_t bit = u % 8;

      return (sprite[i] & (1 << (7 - bit))) != 0;
    }

    bool sample(const float u, const float v) const
    {
      return get(
        static_cast<uint8_t>(u * width),
        static_cast<uint8_t>(v * height)
      );
    }
  };

  enum class entity_type
  {
    DUMMY,
    NONE
  };

  struct entity_t
  {
    vec2d_t pos;
    entity_type type;
  };

  struct world_t
  {
    const uint8_t *data;
    const uint8_t width; // must be multiple of 8
    const uint8_t height;
    const player_t spawn_point;
    const vec2d_t exit_point;

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

public:
  struct state_t
  {
    uint8_t world;
    player_t player;
    entity_t entities[8];
  };

private:
  static void update_player(const world_t *world, player_t &player);

  static void load_world(const world_t *world, player_t &player);
  static bool check_world_exit(const world_t *world, const player_t &player);

  static void draw_entities(entity_t *entities, const uint8_t count, const player_t &player);
  static void draw_world(const world_t *world, const player_t &player);
  static void draw_world_to_console(const world_t *world, const player_t &player);

  const static world_t WORLDS[1];
  static const world_t* get_world(const uint8_t id);

  const static entity_sprite_t ENTITY_SPRITES[static_cast<size_t>(entity_type::NONE)];
  static const entity_sprite_t* get_entity_sprite(const entity_type type);
};

extern MazeGame maze_game;
