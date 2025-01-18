#pragma once

#include "../types.h"
#include "assert.h"

class MazeGame : MarlinGame
{
public:
  static void enter_game();
  static void game_screen();

public:
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

    vec2d_t operator/(const float scalar) const
    {
      return vec2d_t::from(
        x / scalar,
        y / scalar
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

    vec2d_t normalized() const
    {
      return *this / magnitude();
    }

    static vec2d_t from(const float x, const float y)
    {
      return { x, y };
    }
  };

  struct bitmap_t
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
    float rotation; // rotation in radians
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

  struct world_loading_zone_t
  {
    // position of this zone
    uint8_t x;
    uint8_t y;

    // target loading zone to warp to
    // only used when flags.can_warp is true
    uint8_t target_world;
    uint8_t target_zone;

    struct
    {
      // does this zone warp the player on entry?
      // if false, this is only a target for another zone
      bool can_warp : 1;

      // update player rotation on warp
      // - 0: don't update rotation
      // - 1: add 90 degrees
      // - 2: add 180 degrees
      // - 3: add -90 degrees
      uint8_t update_rotation : 2;

      // does this zone trigger game end?
      // only valid if can_warp is true
      bool is_game_exit : 1;
    } flags;
  };

  struct world_t
  {
    const bitmap_t map;
    const world_loading_zone_t *loading_zones;
    const uint8_t loading_zone_count;
  };

  struct entity_info_t
  {
    const bitmap_t sprite;
    const void (*behavior)(const world_t *world, const player_t &player, entity_t &self);
  };

  struct state_t
  {
    uint8_t world;
    player_t player;
    entity_t entities[8];
  };

private:
  static const world_t* load_world(const world_loading_zone_t &zone, player_t &player);
  static bool check_loading_zone(const world_t *world, const player_t &player, world_loading_zone_t &zone);

  static void update_player(const world_t *world, player_t &player);
  static void update_entities(const world_t *world, const player_t &player, entity_t *entities, const uint8_t count);

  static void draw_entities(entity_t *entities, const uint8_t count, const player_t &player, float *depth_buffer);
  static void draw_world(const world_t *world, const player_t &player, float *depth_buffer);
  
  static void draw_to_console(const world_t *world, const player_t &player, const entity_t *entities, const uint8_t entity_count);

  const static world_t WORLDS[2];
  static const world_t* get_world(const uint8_t id);

  const static entity_info_t ENTITIES[static_cast<size_t>(entity_type::NONE)];
  static const entity_info_t* get_entity_info(const entity_type type);
};

extern MazeGame maze_game;
