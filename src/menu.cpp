#include "game/game.h"

MenuGame menu;

const struct
{
  const char *name;
  void (*enter_game)();
} games[] = {
    {"Flappy", flappy.enter_game},
    {"Sn4k3", snake.enter_game},
    {"Brickout", brickout.enter_game},
    {"Invaders", invaders.enter_game},
};
constexpr int num_games = sizeof(games) / sizeof(games[0]);

void MenuGame::enter_game()
{
  init_game(1, game_screen);
}

#include <iostream>

void MenuGame::game_screen()
{
  static int selected_game_index = 0;

  if (game_frame())
    do
    {
      if (ui.encoderPosition > 0)
      {
        selected_game_index = (selected_game_index + 1) % num_games;
        ui.encoderPosition = 0;
      }
      else if (ui.encoderPosition < 0)
      {
        selected_game_index = (selected_game_index - 1 + num_games) % num_games;
        ui.encoderPosition = 0;

        _BUZZ(500, 2000);
      }

      if (ui.use_click())
      {
        games[selected_game_index].enter_game();
      }
    } while (0);

  frame_start();
  set_color(color::WHITE);

  for (int i = 0; i < num_games; i++)
  {
    bool is_selected = i == selected_game_index;

    if (is_selected)
    {
      draw_box(5, i * 15, 10, 10);
    }
    else
    {
      draw_frame(5, i * 15, 10, 10);
    }
    draw_string(20, (i * 15) + 5, games[i].name);
  }

  frame_end();
}
