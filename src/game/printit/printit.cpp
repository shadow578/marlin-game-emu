#include <iostream> // TODO testing

#if 1

#include "../game.h"

// size of one block, squared
constexpr game_dim_t BLOCK_SIZE = 3;

// location of the games print bed (top left corner)
constexpr game_dim_t BED_X = ((GAME_WIDTH - (PRINTIT_BED_WIDTH * BLOCK_SIZE)) / 2);
constexpr game_dim_t BED_Y = ((GAME_HEIGHT - (PRINTIT_BED_HEIGHT * BLOCK_SIZE)) / 2);

// location of the score display (top left corner)
constexpr game_dim_t SCORE_X = (PRINTIT_BED_WIDTH + (BLOCKS_BOARD_WIDTH * BLOCK_SIZE) + 3);
constexpr game_dim_t SCORE_Y = PRINTIT_BED_HEIGHT;

// location of the player's nozzle
constexpr game_dim_t PLAYER_Y = PRINTIT_BED_HEIGHT - 1;

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_ACTIVE 1

#define STATE marlin_game_data.printit

void PrintItGame::enter_game()
{
  init_game(GAME_STATE_ACTIVE, game_screen);

  // reset state
  STATE.bed.clear();
  STATE.player_x = PRINTIT_BED_WIDTH / 2;

  // ensure no falling block is active
  STATE.falling.active = false;
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
    if (handle_player_input(STATE.bed, STATE.falling, STATE.player_x))
    {
      // spawn a new falling block
      if (!spawn_falling(STATE.bed, STATE.player_x, PLAYER_Y, STATE.falling))
      {
        // game over, no space for new block
        game_state = GAME_STATE_GAME_OVER;
      }
    }

    if (handle_falling_gravity(STATE.bed, STATE.falling))
    {
      // landed on something, commit the falling block
      commit_falling(STATE.falling, STATE.bed);

      // set falling block to inactive
      STATE.falling.active = false;
    }
  }

  frame_start();
  draw_bed(BED_X, BED_Y, STATE.bed);
  draw_player(STATE.player_x, PLAYER_Y);
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

bool PrintItGame::handle_player_input(const bed_t &bed, falling_t &falling, uint8_t &player_x)
{
  // record position before update
  const uint8_t oldX = player_x;
  bool dirty = false;

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
  if (dirty && bed.check_collision(player_x, PLAYER_Y) != 0)
  {
    player_x = oldX;
  }

  // spawn a new falling block when clicking and allowed
  return (!falling.active && ui.use_click());
}

bool PrintItGame::handle_falling_gravity(const bed_t &bed, falling_t &falling)
{
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

bool PrintItGame::spawn_falling(const bed_t &bed, const uint8_t x, const uint8_t y, falling_t &falling)
{
  if (bed.check_collision(x, y) == 0)
  {
    falling.x = x;
    falling.y = y;
    falling.active = true;
    return true;
  }
  {
    // spawning not possible!
    falling.active = false;
    return false;
  }
}

void PrintItGame::commit_falling(const falling_t &falling, bed_t &bed)
{
  if (!falling.active)
  {
    return;
  }

  bed.set(falling.x, falling.y, true);
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

void PrintItGame::draw_player(const uint8_t x, const uint8_t y)
{
  set_color(color::BLUE);
  draw_frame(x, y, BLOCK_SIZE, BLOCK_SIZE);
}

void PrintItGame::draw_falling(const falling_t &falling)
{
  set_color(color::RED);
  draw_box(BED_X + (falling.x * BLOCK_SIZE),
           BED_Y + (falling.y * BLOCK_SIZE),
           BLOCK_SIZE,
           BLOCK_SIZE);
}

#endif // MARLIN_PRINTIT
