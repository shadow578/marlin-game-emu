#include <iostream> // TODO testing

#if 1

#include "../game.h"

// how fast the player accelerates towards the ground
constexpr fixed_t GRAVITY = FTOF(0.7f);

// how fast obstacles move towards the player
constexpr fixed_t OBSTACLE_SPEED = FTOF(1.5f);

// height of the ground
constexpr fixed_t GROUND_HEIGHT = FTOF(10);

// position of the player
constexpr fixed_t PLAYER_X = FTOF(10);

// position of obstacles
constexpr fixed_t OBSTACLE_Y = GROUND_HEIGHT;

// how strong the player jumps
constexpr fixed_t JUMP_STRENGTH = FTOF(6);

// minium x distance between obstacles
constexpr fixed_t OBSTACLE_DISTANCE = FTOF(GAME_WIDTH / 3);

// position of the score display
constexpr game_dim_t SCORE_X = 2;
constexpr game_dim_t SCORE_Y = GAME_HEIGHT - 2;

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
  STATE.player.y_position = GROUND_HEIGHT + PLAYER_INFO.height;
  STATE.player.y_velocity = FTOF(0);

  for (auto &obstacle : STATE.obstacles)
  {
    obstacle.type = obstacle_type::NONE;
  }
  STATE.last_spawned_obstacle_index = 0xff;

  score = 0;
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
    update_world(STATE);

    score++;

    // spawn new obstacle if needed
    if (STATE.last_spawned_obstacle_index == 0xff || (STATE.obstacles[STATE.last_spawned_obstacle_index].x < (FTOF(GAME_WIDTH) - OBSTACLE_DISTANCE)))
    {
      STATE.last_spawned_obstacle_index = 0;
      for (auto &obstacle : STATE.obstacles)
      {
        if (obstacle.type == obstacle_type::NONE)
        {
          spawn_obstacle(obstacle);
          break;
        }

        STATE.last_spawned_obstacle_index++;
      }
    }
  }

  frame_start();
  draw_ground(0);
  draw_player(STATE.player);
  for (const auto &obstacle : STATE.obstacles)
  {
    draw_obstacle(obstacle);
  }

  // draw the score display
  set_color(color::WHITE);
  draw_string(
      X_TO_SCREEN(SCORE_X),
      Y_TO_SCREEN(SCORE_Y),
      "Score:");
  draw_int(
      X_TO_SCREEN(SCORE_X + (GAME_FONT_WIDTH * 7)),
      Y_TO_SCREEN(SCORE_Y),
      score);

  frame_end();
}

void DinoGame::on_jump()
{
  _BUZZ(5, 280);
}

void DinoGame::on_obstacle_despawn(obstacle_t &obstacle)
{
  score += 10;

  std::cout << "Obstacle despawn type: " << static_cast<int>(obstacle.type) << std::endl;
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
    on_jump();
  }

  // no need to update physics if player is grounded
  if (player.grounded())
    return;

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
    player.y_position = GROUND_HEIGHT + player_box.height;
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
      on_obstacle_despawn(obstacle);
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

  for (uint8_t i = 0; i < 10; i++)
  {
    slot.type = static_cast<obstacle_type>(random(0, static_cast<int>(obstacle_type::NONE)));

    // ensure this type can spawn
    if (get_obstacle_info(slot.type)->min_score <= score)
      break;

    slot.type = obstacle_type::NONE;
  }

  if (slot.type == obstacle_type::NONE)
  {
    std::cout << "Failed to spawn obstacle" << std::endl;
  }
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
  box.width = PLAYER_INFO.width;
  box.height = PLAYER_INFO.height;
}

void DinoGame::get_obstacle_bounding_box(const obstacle_t &obstacle, aabb_t &box)
{
  const obstacle_info_t *sprite = get_obstacle_info(obstacle.type);

  box.x = obstacle.x;
  box.y = OBSTACLE_Y + sprite->height + sprite->y_offset;
  box.width = sprite->width;
  box.height = sprite->height;
}

void DinoGame::get_ground_bounding_box(aabb_t &box)
{
  box.x = FTOF(0);
  box.y = GROUND_HEIGHT;
  box.width = FTOF(GAME_WIDTH);
  box.height = FTOF(2);
}

const DinoGame::obstacle_info_t *DinoGame::get_obstacle_info(obstacle_type type)
{
  return &OBSTACLE_INFO[static_cast<int>(type)];
}

#endif // MARLIN_DINO
