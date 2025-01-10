#include <iostream> // TODO testing

#if 1

#include "../game.h"

struct sprite_t
{
  // bitmap_t shape;
  fixed_t width;
  fixed_t height;
};

constexpr sprite_t PLAYER_SPRITE = {
  .width = FTOF(5),
  .height = FTOF(5),
};

// how fast the player accelerates towards the ground
constexpr fixed_t GRAVITY = FTOF(0.1f);

// how fast obstacles move towards the player
constexpr fixed_t OBSTACLE_SPEED = FTOF(0.1f);

// height of the ground
constexpr fixed_t GROUND_HEIGHT = FTOF(10);

// idle position of the player
constexpr fixed_t PLAYER_X = 10;
constexpr fixed_t PLAYER_IDLE_Y = GROUND_HEIGHT + PLAYER_SPRITE.height;

// how strong the player jumps
constexpr fixed_t JUMP_STRENGTH = FTOF(3);

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_WAIT_ON_USER 1
#define GAME_STATE_RUNNING 2

#define STATE (marlin_game_data.dino)

#define X_TO_SCREEN(x) (x)
#define Y_TO_SCREEN(y) (GAME_HEIGHT - y)

void DinoGame::enter_game()
{
  init_game(GAME_STATE_WAIT_ON_USER, game_screen);

  // reset state
  STATE.player.y_position = PLAYER_IDLE_Y;
  STATE.player.y_velocity = 0;

  for (auto &obstacle : STATE.obstacles)
  {
    obstacle.type = obstacle_type::NONE;
  }
}

void DinoGame::game_screen()
{
  if (game_state == GAME_STATE_GAME_OVER)
  {
    if (ui.use_click())
      exit_game();
  }
  else if (game_state == GAME_STATE_WAIT_ON_USER)
  {
    game_state = GAME_STATE_RUNNING;
  }
  else if (game_state == GAME_STATE_RUNNING)
  {
    update_player(STATE.player);
  }

  frame_start();
  draw_player(STATE.player);

  for (const auto &obstacle : STATE.obstacles)
  {
    draw_obstacle(obstacle);
  }

  draw_ground(0);

  frame_end();
}

void DinoGame::update_player(player_t &player)
{
  // use click first to avoid buffering clicks
  if (ui.use_click() && player.grounded())
  {
    player.y_velocity += JUMP_STRENGTH;
  }

  // no need to update physics if player is grounded
  if (player.grounded()) return;

  player.y_velocity -= GRAVITY;
  player.y_position += player.y_velocity;

  std::cout << "Player Y: " << PTOF(player.y_position) << "; Player V: " << PTOF(player.y_velocity) << std::endl;

  // did player fall below the ground?
  if (player.y_position < PLAYER_IDLE_Y)
  {
    player.y_position = PLAYER_IDLE_Y;
    player.set_grounded();
  }
}

void DinoGame::handle_world_movement()
{
}

void DinoGame::draw_player(const player_t &player)
{
  set_color(color::BLUE);
  draw_box(X_TO_SCREEN(PLAYER_X),
           Y_TO_SCREEN(FTOB(player.y_position)),
           FTOB(PLAYER_SPRITE.width),
           FTOB(PLAYER_SPRITE.height));
}

void DinoGame::draw_obstacle(const obstacle_t &obstacle)
{
  if (obstacle.type == obstacle_type::NONE)
    return;

  set_color(color::RED);
  draw_box(X_TO_SCREEN(FTOB(obstacle.x)),
           Y_TO_SCREEN(FTOB(obstacle.y)),
           5,
           5);
}

void DinoGame::draw_ground(const uint8_t offset)
{
  // TODO add variation to the ground using offset
  set_color(color::WHITE);
  draw_hline(0,
           Y_TO_SCREEN(FTOB(GROUND_HEIGHT)),
           GAME_WIDTH);
}

#endif // MARLIN_DINO
