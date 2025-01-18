#include "maze.h"

const uint8_t WORLD1_DATA[] PROGMEM = 
{
  0b11111111, 0b11111111, // ################
  0b10000001, 0b00000001, // #0     #       #
  0b10110001, 0b00111101, // # ##   #  #### #
  0b10000001, 0b00000101, // #      #     #2#
  0b10000000, 0b00001101, // #           ##1#
  0b11111111, 0b11111111, // ################
};

const MazeGame::world_loading_zone_t WORLD1_LOADING_ZONES[] PROGMEM =
{
  { // 0: SPAWN
    .x = 1,
    .y = 1,
  },
  { // 1: to world 2
    .x = 14,
    .y = 4,
    .target_world = 1,
    .target_zone = 0,
    .flags =
    {
      .can_warp = true,
    }
  },
  { // 2: return from world 2
    .x = 14,
    .y = 3,
  }
};

const uint8_t WORLD2_DATA[] PROGMEM = 
{
  0b11111111, // ########
  0b10110001, // #1##2  #
  0b10111101, // # #### #
  0b10000001, // #      #
  0b11111111, // ########
};

const MazeGame::world_loading_zone_t WORLD2_LOADING_ZONES[] PROGMEM =
{
  { // 0: Entry from world 1
    .x = 1,
    .y = 1,
  },
  { // 1: Warp back to world 1
    .x = 4,
    .y = 1,
    .target_world = 0,
    .target_zone = 2,
    .flags =
    {
      .can_warp = true,
      .update_rotation = 3, // -90
    }
  }
};

const MazeGame::world_t MazeGame::WORLDS[] =
{
  {
    .map =
    {
      .data = WORLD1_DATA,
      .width = 16,
      .height = 6
    },
    .loading_zones = WORLD1_LOADING_ZONES,
    .loading_zone_count = COUNT(WORLD1_LOADING_ZONES),
  },
  {
    .map = 
    {
      .data = WORLD2_DATA,
      .width = 8,
      .height = 5
    },
    .loading_zones = WORLD2_LOADING_ZONES,
    .loading_zone_count = COUNT(WORLD2_LOADING_ZONES),
  }
};

const MazeGame::world_t* MazeGame::get_world(const uint8_t id)
{
  return &WORLDS[id];
}
