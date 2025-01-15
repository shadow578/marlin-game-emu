#include "maze.h"

const uint8_t WORLD1_DATA[] PROGMEM = 
{
  0b11111111, 0b11111111,
  0b10000001, 0b00011001,
  0b10110001, 0b01111001,
  0b10000000, 0b00000001,
  0b10000000, 0b00001101,
  0b11111111, 0b11111111,
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
    .spawn_point = 
    { 
      .pos = { 1, 1 },
      .rotation = 0
    },
    .exit_point = { 14, 4 }
  }
};

const MazeGame::world_t* MazeGame::get_world(const uint8_t id)
{
  return &WORLDS[id];
}
