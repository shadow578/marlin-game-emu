#include "maze.h"

const uint8_t DUMMY_SPRITE[] PROGMEM = 
{
  0b00000111,
  0b10000111,
  0b01111100,
  0b00111000,
  0b00100000,
};

const MazeGame::entity_info_t MazeGame::ENTITIES[] PROGMEM =
{
  { // DUMMY
    .sprite =
    {
      .data = DUMMY_SPRITE,
      .width = 8,
      .height = 5
    }
  }
};

const MazeGame::entity_info_t* MazeGame::get_entity_info(const entity_type type)
{
  return &ENTITIES[static_cast<uint8_t>(type)];
}
