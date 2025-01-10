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
constexpr fixed_t GRAVITY = FTOF(0.7f);

// how fast obstacles move towards the player
constexpr fixed_t OBSTACLE_SPEED = FTOF(1.5f);

// height of the ground
constexpr fixed_t GROUND_HEIGHT = FTOF(10);

// position of the player
constexpr fixed_t PLAYER_X = FTOF(10);
constexpr fixed_t PLAYER_IDLE_Y = GROUND_HEIGHT + PLAYER_SPRITE.height;

// position of obstacles
constexpr fixed_t OBSTACLE_Y = GROUND_HEIGHT;

// how strong the player jumps
constexpr fixed_t JUMP_STRENGTH = FTOF(6);

// how many frames between spawning obstacles initially
constexpr uint8_t SPAWN_DELAY = 60;

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

  // TODO testing
  STATE.obstacles[0].x = FTOF(GAME_WIDTH - 10);
  STATE.obstacles[0].type = obstacle_type::CACTUS_SMALL;
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
    STATE.ticks = 0;
  }
  else if (game_state == GAME_STATE_RUNNING)
  {
    update_player(STATE.player);
    update_world(STATE);

    STATE.ticks++;

    if (STATE.ticks > SPAWN_DELAY)
    {
      STATE.ticks = 0;

      for (auto &obstacle : STATE.obstacles)
      {
        if (obstacle.type == obstacle_type::NONE)
        {
          spawn_obstacle(obstacle);
          break;
        }
      }
    }
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

void DinoGame::on_collision(const player_t &player, const obstacle_t &obstacle)
{
  game_state = GAME_STATE_GAME_OVER;
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

  // did player fall into the ground?
  aabb_t player_box;
  aabb_t ground_box;
  get_player_bounding_box(player, player_box);
  get_ground_bounding_box(ground_box);
  if (player_box.intersects(ground_box))
  {
    player.y_position = PLAYER_IDLE_Y;
    player.set_grounded();
  }
}

void DinoGame::update_world(state_t &state)
{
  aabb_t player_box;
  get_player_bounding_box(state.player, player_box);

  for (auto &obstacle : state.obstacles)
  {
    if (obstacle.type == obstacle_type::NONE)
      continue;

    obstacle.x -= OBSTACLE_SPEED;

    if (obstacle.x < FTOF(0))
    {
      //spawn_obstacle(obstacle);
      obstacle.type = obstacle_type::NONE;
      continue;
    }

    // test collision
    aabb_t obstacle_box;
    get_obstacle_bounding_box(obstacle, obstacle_box);
    if (player_box.intersects(obstacle_box))
    {
      on_collision(state.player, obstacle);
    }
  }
}

void DinoGame::spawn_obstacle(obstacle_t &slot)
{
  slot.x = FTOF(GAME_WIDTH);
  slot.type = static_cast<obstacle_type>(random(0, static_cast<int>(obstacle_type::NONE)));
}

void DinoGame::draw_player(const player_t &player)
{
  set_color(color::BLUE);

  aabb_t box;
  get_player_bounding_box(player, box);
  draw_aabb(box);
}

void DinoGame::draw_obstacle(const obstacle_t &obstacle)
{
  if (obstacle.type == obstacle_type::NONE)
    return;

  set_color(color::RED);
  
  aabb_t box;
  get_obstacle_bounding_box(obstacle, box);
  draw_aabb(box);
}

void DinoGame::draw_ground(const uint8_t offset)
{
  // TODO add variation to the ground using offset
  set_color(color::WHITE);
  
  aabb_t box;
  get_ground_bounding_box(box);
  draw_aabb(box);
}

void DinoGame::draw_aabb(const aabb_t &box)
{
  draw_box(X_TO_SCREEN(FTOB(box.x)),
           Y_TO_SCREEN(FTOB(box.y)),
           FTOB(box.width),
           FTOB(box.height));
}

void DinoGame::get_player_bounding_box(const player_t &player, aabb_t &box)
{
  box.x = PLAYER_X;
  box.y = player.y_position;
  box.width = PLAYER_SPRITE.width;
  box.height = PLAYER_SPRITE.height;
}

void DinoGame::get_obstacle_bounding_box(const obstacle_t &obstacle, aabb_t &box)
{
  box.x = obstacle.x;
  box.y = OBSTACLE_Y + FTOF(5);
  box.width = FTOF(5);
  box.height = FTOF(5);
}

void DinoGame::get_ground_bounding_box(aabb_t &box)
{
  box.x = FTOF(0);
  box.y = GROUND_HEIGHT;
  box.width = FTOF(GAME_WIDTH);
  box.height = FTOF(2);
}

#endif // MARLIN_DINO
