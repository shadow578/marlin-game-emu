#include "game/game.h"
#include "game_renderer.h"

MenuGame menu;

static void toggle_color()
{
  set_color_enabled(!get_color_enabled());
}

const struct
{
  const char *name;
  void (*run)();
} actions[] = {
    {"PrintIt", printit_game.enter_game},
    //{"Blocks", blocks_game.enter_game},
    //{"Flappy", flappy.enter_game},
    //{"Tennis", tennis.enter_game},
    //{"Sn4k3", snake.enter_game},
    //{"Brickout", brickout.enter_game},
    //{"Invaders", invaders.enter_game},
    {"Toggle Color", toggle_color},
};
constexpr int num_games = sizeof(actions) / sizeof(actions[0]);

void MenuGame::enter_game()
{
  init_game(1, game_screen);
}

#include <iostream>

void MenuGame::game_screen()
{
  static int selected_action_index = 0;

  if (game_frame())
    do
    {
      if (ui.encoderPosition > 0)
      {
        selected_action_index = (selected_action_index + 1) % num_games;
        ui.encoderPosition = 0;
      }
      else if (ui.encoderPosition < 0)
      {
        selected_action_index = (selected_action_index - 1 + num_games) % num_games;
        ui.encoderPosition = 0;

        _BUZZ(500, 2000);
      }

      if (ui.use_click())
      {
        actions[selected_action_index].run();
      }
    } while (0);

  frame_start();

  for (int i = 0; i < num_games; i++)
  {
    bool is_selected = i == selected_action_index;

    if (is_selected)
    {
      set_color(color::GREEN);
      draw_box(5, i * 15, 10, 10);
    }
    else
    {
      set_color(color::WHITE);
      draw_frame(5, i * 15, 10, 10);
    }

    draw_string(20, (i * 15) + 5, actions[i].name);
  }

  frame_end();
}
