#pragma once

#include "game/types.h"

class MenuGame : MarlinGame
{
public: 
  static void enter_game(),
              game_screen();
};

extern MenuGame menu;
