#include "dino.h"

const DinoGame::player_info_t DinoGame::PLAYER_INFO = 
{
  .width = FTOF(5),
  .height = FTOF(5),
};

const DinoGame::obstacle_info_t DinoGame::OBSTACLE_INFO[static_cast<int>(obstacle_type::NONE)] = 
{
  { // CACTUS_SMALL
    .width = FTOF(3),
    .height = FTOF(5),
    .y_offset = FTOF(0),
    .min_score = 0,
  },
  { // CACTUS_LARGE
    .width = FTOF(8),
    .height = FTOF(5),
    .y_offset = FTOF(0),
    .min_score = 0,
  },
  { // BIRD
    .width = FTOF(5),
    .height = FTOF(5),
    .y_offset = FTOF(8),
    .min_score = 50,
  },
};
