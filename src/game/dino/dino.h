#pragma once

#include "../types.h"
#include "assert.h"

class DinoGame : MarlinGame
{
public:
  static void enter_game();
  static void game_screen();

private:
  struct player_t
  {
    fixed_t y_position;
    fixed_t y_velocity;

    void set_grounded()
    {
      y_velocity = FTOF(0);
    }

    bool grounded() const
    {
      return y_velocity == FTOF(0);
    }
  };

  enum class obstacle_type 
  {
    NONE,
    CACTUS_SMALL,
    CACTUS_LARGE,
    BIRD
  };

  struct obstacle_t
  {
    fixed_t x;
    fixed_t y;
    obstacle_type type;
  };

public:
  struct state_t
  {
    player_t player;
    obstacle_t obstacles[4];
  };

private:
  static void update_player(player_t &player);
  static void handle_world_movement();

  static void draw_player(const player_t &player);
  static void draw_obstacle(const obstacle_t &obstacle);
  static void draw_ground(const uint8_t offset);
};

extern DinoGame dino_game;
