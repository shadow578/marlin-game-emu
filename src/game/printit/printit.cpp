#include <iostream> // TODO testing

#if 1

#include "../game.h"

// size of one block, squared
constexpr game_dim_t BLOCK_SIZE = 3;

// helpers for print bed geometry calculations
constexpr game_dim_t BED_SCREEN_WIDTH = PRINTIT_BED_WIDTH * BLOCK_SIZE;
constexpr game_dim_t BED_SCREEN_HEIGHT = PRINTIT_BED_HEIGHT * BLOCK_SIZE;

// location of the player's print bed
constexpr game_dim_t BED_X = GAME_WIDTH - BED_SCREEN_WIDTH - 2;
constexpr game_dim_t BED_Y = (GAME_HEIGHT - BED_SCREEN_HEIGHT) / 2;

// location of the target print bed
constexpr game_dim_t TARGET_BED_X = 2;
constexpr game_dim_t TARGET_BED_Y = BED_Y;

// location of the score display (top left corner)
constexpr game_dim_t SCORE_X = TARGET_BED_X + BED_SCREEN_WIDTH + 2;
constexpr game_dim_t SCORE_Y = BED_Y;

// location of the player's nozzle
constexpr game_dim_t PLAYER_Y = 0;

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_ACTIVE 1

#define STATE marlin_game_data.printit

PrintItGame::bed_t PrintItGame::target_bed;

void PrintItGame::enter_game()
{
  init_game(GAME_STATE_ACTIVE, game_screen);

  // reset state
  STATE.bed.clear();
  STATE.falling.x = PRINTIT_BED_WIDTH / 2;
  STATE.falling.y = PLAYER_Y;
  STATE.falling.is_falling = false;
  STATE.level = 0;

  // set up the first level
  levels[STATE.level].init(target_bed);
}

void PrintItGame::game_screen()
{
  if (game_state == GAME_STATE_GAME_OVER)
  {
    if (ui.use_click())
      exit_game();
  }
  else if (game_state == GAME_STATE_ACTIVE)
  {
    if (handle_player_input(STATE.bed, STATE.falling))
    {
      STATE.falling.is_falling = true;
    }    

    if (handle_falling_gravity(STATE.bed, STATE.falling))
    {
      // landed on something, commit the falling block
      commit_falling(STATE.falling, STATE.bed);
      on_falling_committed(STATE.falling);

      // set falling block to player input
      STATE.falling.is_falling = false;

      // check level status
      const uint8_t status = get_level_status(STATE.bed);
      if (status == 1)
      {
        on_level_completed();
      }
      else if (status == 2)
      {
        game_state = GAME_STATE_GAME_OVER;
      }
    }
  }

  frame_start();
  draw_bed(TARGET_BED_X, TARGET_BED_Y, target_bed);

  draw_bed(BED_X, BED_Y, STATE.bed);
  draw_falling(STATE.falling);

  // draw score
  set_color(color::WHITE);
  draw_string(SCORE_X, SCORE_Y, "Score:");
  draw_int(SCORE_X, SCORE_Y + GAME_FONT_ASCENT, score);

  // draw game over screen
  if (game_state == GAME_STATE_GAME_OVER)
    draw_game_over();

  frame_end();
}

void PrintItGame::on_falling_committed(const falling_t &falling)
{
  score++;
}

void PrintItGame::on_level_completed()
{
  STATE.level++;
  if (STATE.level < PRINTIT_LEVEL_COUNT)
  {
    levels[STATE.level].init(target_bed);
  }
  else
  {
    game_state = GAME_STATE_GAME_OVER;
  }
}

bool PrintItGame::handle_player_input(const bed_t &bed, falling_t &falling)
{
  // ignore player input while last block is falling
  if (falling.is_falling) return false;

  // record position before update
  const falling_t old = falling;
  bool dirty = false;

  // ensure falling block is rendered at player's y position
  falling.y = PLAYER_Y;

  // update left/right movement by encoder
  if (ui.encoderPosition > 0)
  {
    falling.x++;
    dirty = true;
  }
  else if (ui.encoderPosition < 0)
  {
    falling.x--;
    dirty = true;
  }
  ui.encoderPosition = 0;

  // block movement if it would collide
  if (dirty && bed.check_collision(falling.x, falling.y) != 0)
  {
    falling = old;
    return false;
  }

  // spawn a new falling block when clicking
  return ui.use_click();
}

bool PrintItGame::handle_falling_gravity(const bed_t &bed, falling_t &falling)
{
  // ignore if not falling
  if (!falling.is_falling) return false;

  // record position before update
  const uint8_t oldY = falling.y;

  // make the block fall
  falling.y++;

  // undo falling and commit if collision detected
  if (bed.check_collision(falling.x, falling.y) != 0)
  {
    falling.y = oldY;
    return true;
  }

  return false;
}

void PrintItGame::commit_falling(const falling_t &falling, bed_t &bed)
{
  if (!falling.is_falling)
  {
    return;
  }

  bed.set(falling.x, falling.y, true);
}

uint8_t PrintItGame::get_level_status(const bed_t &bed)
{
  bool all_blocks_match = true;

  for (uint8_t x = 0; x < PRINTIT_BED_WIDTH; x++)
  {
    for (uint8_t y = 0; y < PRINTIT_BED_HEIGHT; y++)
    {
      const bool target = target_bed.get(x, y);
      const bool player = bed.get(x, y);

      // game over if there are any blocks in the players bed that are not in the target bed
      if (player && !target)
      {
        return 2;
      }

      // check if all blocks match
      if (all_blocks_match && target != player)
      {
        all_blocks_match = false;
      }
    }
  }

  return all_blocks_match ? 1 : 0;
}

void PrintItGame::draw_bed(const uint8_t screen_x, const uint8_t screen_y, const bed_t &bed)
{
  set_color(color::CYAN);
  for (uint8_t x = 0; x < PRINTIT_BED_WIDTH; x++)
  {
    for (uint8_t y = 0; y < PRINTIT_BED_HEIGHT; y++)
    {
      if (bed.get(x, y))
      {
        draw_box(screen_x + (x * BLOCK_SIZE),
                 screen_y + (y * BLOCK_SIZE),
                 BLOCK_SIZE,
                 BLOCK_SIZE);
      }
    }
  }

  // draw outline
  set_color(color::WHITE);
  draw_frame(screen_x - 1,
             screen_y - 1,
             (PRINTIT_BED_WIDTH * BLOCK_SIZE) + 2,
             (PRINTIT_BED_HEIGHT * BLOCK_SIZE) + 2);
}

void PrintItGame::draw_falling(const falling_t &falling)
{
  set_color(color::RED);
  draw_frame(BED_X + (falling.x * BLOCK_SIZE),
           BED_Y + (falling.y * BLOCK_SIZE),
           BLOCK_SIZE,
           BLOCK_SIZE);
}

#endif // MARLIN_PRINTIT
