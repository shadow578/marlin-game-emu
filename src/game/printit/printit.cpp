#include <iostream> // TODO testing

#if 1

#include "../game.h"

// for how long the target bed is shown after level start
// set to 0 to always show the target bed
constexpr millis_t TARGET_SHOW_TIME = 0;

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

// location of the level name display and score display
constexpr game_dim_t STATS_X = TARGET_BED_X + BED_SCREEN_WIDTH + 2;
constexpr game_dim_t STATS_Y = BED_Y;

// location of the player's nozzle
constexpr game_dim_t PLAYER_Y = PRINTIT_BED_HEIGHT - 1;

// location of game message (game over, level clear, game finished) box
constexpr game_dim_t MESSAGE_WIDTH = (GAME_FONT_WIDTH * 22) + 2;
constexpr game_dim_t MESSAGE_HEIGHT = (GAME_FONT_ASCENT * 2) + 2;
constexpr game_dim_t MESSAGE_X = (GAME_WIDTH - MESSAGE_WIDTH) / 2;
constexpr game_dim_t MESSAGE_Y = (GAME_HEIGHT - MESSAGE_HEIGHT) / 2;

#define MESSAGE_GAME_OVER "Game Over"
#define MESSAGE_LEVEL_CLEAR "Level Clear"
#define MESSAGE_FINISHED "Game Finished"
#define MESSAGE_WELCOME "Welcome to PrintIt!"

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_ACTIVE 1
#define GAME_STATE_LEVEL_CLEAR 2
#define GAME_STATE_FINISHED 3
#define GAME_STATE_WELCOME 4

#define STATE marlin_game_data.printit

PrintItGame::bed_t PrintItGame::target_bed;
const char* PrintItGame::level_name = nullptr;

void PrintItGame::enter_game()
{
  init_game(GAME_STATE_WELCOME, game_screen);
  load_level(0);
}

void PrintItGame::game_screen()
{
  bool do_draw_message_box = false;
  if (game_state == GAME_STATE_GAME_OVER || game_state == GAME_STATE_FINISHED)
  {
    if (ui.use_click())
      exit_game();

    do_draw_message_box = true;
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
      STATE.falling.y = PLAYER_Y;

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
  else if (game_state == GAME_STATE_LEVEL_CLEAR)
  {
    do_draw_message_box = true;

    if (ui.use_click())
    {
      load_level(STATE.level++);
      game_state = GAME_STATE_ACTIVE;
    }
  }
  else if (game_state == GAME_STATE_WELCOME)
  {
    do_draw_message_box = true;

    if (ui.use_click())
    {
      game_state = GAME_STATE_ACTIVE;
    }
  }

  frame_start();
  bool show_target_bed = true;
  if (TARGET_SHOW_TIME != 0 && game_state == GAME_STATE_ACTIVE)
  {
    const millis_t now = millis();

    if (STATE.level_start_millis == 0)
    {
      STATE.level_start_millis = now;
    }

    show_target_bed = (now - STATE.level_start_millis) < TARGET_SHOW_TIME;
  }

  set_color(color::GREEN);
  draw_bed(TARGET_BED_X, TARGET_BED_Y, target_bed, show_target_bed);

  set_color(color::CYAN);
  if (game_state == GAME_STATE_WELCOME)
  {
    draw_bed(BED_X, BED_Y, target_bed);
  }
  else 
  {
    draw_bed(BED_X, BED_Y, STATE.bed);
    draw_falling(STATE.falling);
  }

  set_color(color::WHITE);

  // draw level name
  game_dim_t stats_y = STATS_Y;
  draw_string(STATS_X, stats_y, F("Level:"));
  draw_int(STATS_X + (GAME_FONT_WIDTH * 7), stats_y, STATE.level + 1);

  stats_y += GAME_FONT_ASCENT + 2;
  if (level_name != nullptr)
    draw_string(STATS_X, stats_y, level_name);

  // draw score
  stats_y += GAME_FONT_ASCENT + 2;
  stats_y += GAME_FONT_ASCENT + 2;
  draw_string(STATS_X, stats_y, F("Score:"));
  draw_int(STATS_X + (GAME_FONT_WIDTH * 7) , stats_y, score);

  if (do_draw_message_box)
    draw_message_box();

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
    game_state = GAME_STATE_LEVEL_CLEAR;
  }
  else
  {
    game_state = GAME_STATE_FINISHED;
  }
}

bool PrintItGame::handle_player_input(const bed_t &bed, falling_t &falling)
{
  // ignore player input while last block is falling
  if (falling.is_falling)
    return false;

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
  if (!falling.is_falling)
    return false;

  // record position before update
  const uint8_t oldY = falling.y;

  // make the block fall
  falling.y--;

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

void PrintItGame::load_level(const uint8_t level)
{
  assert(level < PRINTIT_LEVEL_COUNT);

  // reset player state
  STATE.bed.clear();
  STATE.falling.x = PRINTIT_BED_WIDTH / 2;
  STATE.falling.y = PLAYER_Y;
  STATE.falling.is_falling = false;

  /// signal that level start was not yet set
  STATE.level_start_millis = 0;

  // set up the level
  STATE.level = level;
  target_bed.clear();
  levels[level].init(target_bed);

  level_name = levels[level].name;
}

void PrintItGame::draw_bed(const uint8_t screen_x, const uint8_t screen_y, const bed_t &bed, const bool draw_blocks)
{
  if (draw_blocks)
  {
    for (uint8_t x = 0; x < PRINTIT_BED_WIDTH; x++)
    {
      for (uint8_t y = 0; y < PRINTIT_BED_HEIGHT; y++)
      {
        if (bed.get(x, y))
        {
          draw_box(screen_x + (x * BLOCK_SIZE),
                   screen_y + (BED_SCREEN_HEIGHT - ((y + 1) * BLOCK_SIZE)),
                   BLOCK_SIZE,
                   BLOCK_SIZE);
        }
      }
    }
  }

  set_color(color::WHITE);
  draw_frame(screen_x - 1,
             screen_y - 1,
             BED_SCREEN_WIDTH + 2,
             BED_SCREEN_HEIGHT + 2);
}

void PrintItGame::draw_falling(const falling_t &falling)
{
  set_color(color::RED);
  draw_frame(BED_X + (falling.x * BLOCK_SIZE),
             BED_Y + (BED_SCREEN_HEIGHT - ((falling.y + 1) * BLOCK_SIZE)),
             BLOCK_SIZE,
             BLOCK_SIZE);
}

void PrintItGame::draw_message_box()
{
  set_color(color::BLACK);
  draw_box(MESSAGE_X, MESSAGE_Y, MESSAGE_WIDTH, MESSAGE_HEIGHT);

  set_color(color::WHITE);
  draw_frame(MESSAGE_X - 1, MESSAGE_Y - 1, MESSAGE_WIDTH + 2, MESSAGE_HEIGHT + 2);

  switch (game_state)
  {
  case GAME_STATE_GAME_OVER:
    draw_string(MESSAGE_X + 1, MESSAGE_Y + 1, F(MESSAGE_GAME_OVER));
    break;
  case GAME_STATE_LEVEL_CLEAR:
    draw_string(MESSAGE_X + 1, MESSAGE_Y + 1, F(MESSAGE_LEVEL_CLEAR));
    break;
  case GAME_STATE_FINISHED:
    draw_string(MESSAGE_X + 1, MESSAGE_Y + 1, F(MESSAGE_FINISHED));
    break;
  case GAME_STATE_WELCOME:
    draw_string(MESSAGE_X + 1, MESSAGE_Y + 1, F(MESSAGE_WELCOME));
    break;
  }
}

#endif // MARLIN_PRINTIT
