#include "maze.h"

const uint8_t DUMMY_SPRITE[] PROGMEM = 
{
  0b00000111,
  0b10000111,
  0b01111100,
  0b00111000,
  0b00100000,
};

static const void DUMMY_BEHAVIOUR(const MazeGame::world_t *world, const MazeGame::player_t &player, MazeGame::entity_t &self)
{
  // move straight towards player
  const MazeGame::vec2d_t target = player.pos + MazeGame::vec2d_t::from(1,1);
  const MazeGame::vec2d_t dir = (target - self.pos).normalized();
  self.pos = self.pos + (dir * 0.01f);
}

const MazeGame::entity_info_t MazeGame::ENTITIES[] PROGMEM =
{
  { // DUMMY
    .sprite =
    {
      .data = DUMMY_SPRITE,
      .width = 8,
      .height = 5
    },
    .behavior = DUMMY_BEHAVIOUR
  }
};

const MazeGame::entity_info_t* MazeGame::get_entity_info(const entity_type type)
{
  return &ENTITIES[static_cast<uint8_t>(type)];
}
