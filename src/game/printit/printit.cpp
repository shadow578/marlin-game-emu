#include <iostream> // TODO testing

#if 1

#include "../game.h"

// for how long the target bed is shown after level start
// set to 0 to always show the target bed
constexpr millis_t TARGET_SHOW_TIME = 0;

// how many milliseconds between each gravity update, ms
// lower causes block to fall faster
#define FALL_SPEED(level) (200 - (level * 10))

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

// location of the level name display
constexpr game_dim_t STATS_X = TARGET_BED_X + BED_SCREEN_WIDTH + 2;
constexpr game_dim_t STATS_Y = BED_Y;

// location of the player's nozzle
constexpr game_dim_t PLAYER_Y = PRINTIT_BED_HEIGHT - 1;

// location of game message (game over, level clear, game finished) box
constexpr game_dim_t MESSAGE_WIDTH = (GAME_FONT_WIDTH * 30) + 2;
constexpr game_dim_t MESSAGE_HEIGHT = (GAME_FONT_ASCENT * 6) + 2;
constexpr game_dim_t MESSAGE_X = (GAME_WIDTH - MESSAGE_WIDTH) / 2;
constexpr game_dim_t MESSAGE_Y = (GAME_HEIGHT - MESSAGE_HEIGHT) / 2;

// scoring thresholds, 0-1000
constexpr int PERFECT_SCORE = 900;
constexpr int PASSING_SCORE = 500;

#define MESSAGE_WELCOME_1 "Take on the role of a 3D printer!"
#define MESSAGE_WELCOME_2 "Print the model to progress"
#define MESSAGE_WELCOME_3 "Click to start"
#define MESSAGE_GAME_OVER_1 "Game Over"
#define MESSAGE_GAME_OVER_2 "You were sent to the scrap"
#define MESSAGE_LEVEL_CLEAR "Level Clear!"
#define MESSAGE_CLEAR_PASSING "You could use some calibration"
#define MESSAGE_CLEAR_PERFECT "A perfect Print!"

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_ACTIVE 1
#define GAME_STATE_LEVEL_CLEAR 2
#define GAME_STATE_FINISHED 3
#define GAME_STATE_WELCOME 4

#define STATE marlin_game_data.printit

PrintItGame::bed_t PrintItGame::target_bed;
const PrintItGame::level_t *PrintItGame::current_level;

void PrintItGame::enter_game()
{
  init_game(GAME_STATE_WELCOME, game_screen);
  load_level(0);
}

void PrintItGame::game_screen()
{
  const millis_t now = millis();

  bool do_draw_message_box = false;
  if (game_state == GAME_STATE_GAME_OVER || game_state == GAME_STATE_FINISHED)
  {
    do_draw_message_box = true;
    
    if (ui.use_click())
      exit_game();
  }
  else if (game_state == GAME_STATE_ACTIVE)
  {
    if (handle_player_input(STATE.bed, STATE.falling))
    {
      STATE.falling.is_falling = true;
    }

    bool commit_block = handle_falling_gravity(STATE.bed, STATE.falling, now, FALL_SPEED(STATE.level));
    if (!commit_block && ui.use_click())
    {
      commit_block = true;
    }
    if (commit_block)
    {
      commit_falling(STATE.falling, STATE.bed);
      on_falling_committed(STATE.falling);

      // set falling block to player input
      STATE.falling.is_falling = false;
      STATE.falling.y = PLAYER_Y;

      // check if the player used up all blocks in the level
      const uint16_t available = target_bed.get_set_blocks();
      const uint16_t placed = STATE.bed.get_set_blocks();
      if (placed >= available)
      {
        on_level_over();
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
  draw_string(STATS_X, stats_y, current_level->name);

  if (do_draw_message_box)
    draw_message_box();

  frame_end();
}

void PrintItGame::on_falling_committed(const falling_t &falling)
{
}

void PrintItGame::on_level_over()
{
  score = calculate_level_score(STATE.bed);
  if (score < PASSING_SCORE)
  {
    game_state = GAME_STATE_GAME_OVER;
    return;
  }

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

bool PrintItGame::handle_falling_gravity(const bed_t &bed, falling_t &falling, const millis_t now, const millis_t fall_speed)
{
  // ignore if not falling
  const bool should_fall = (now - STATE.falling.last_update_millis) > FALL_SPEED(STATE.level);
  if (!falling.is_falling || !should_fall)
    return false;

  // record position before update
  const uint8_t oldY = falling.y;

  // make the block fall
  falling.y--;
  falling.last_update_millis = now;

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

int PrintItGame::calculate_level_score(const bed_t &bed)
{
  int s = 0;
  int ps = 0;
  for (uint8_t x = 0; x < PRINTIT_BED_WIDTH; x++)
  {
    for (uint8_t y = 0; y < PRINTIT_BED_HEIGHT; y++)
    {
      const bool target = target_bed.get(x, y);
      const bool player = bed.get(x, y);

      // update perfect score
      if (target) ps += 10;

      // for every correctly placed block, award 10 points
      if (target && player)
      {
        s += 10;
      }
    }
  }

  std::cout << "s: " << s << " ps: " << ps << std::endl;

  // convert to 0-1000
  return (s * 1000) / ps;
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

  current_level = &levels[level];
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
  {
    game_dim_t y = MESSAGE_Y + 1;
    draw_string(MESSAGE_X + 1, y, F(MESSAGE_GAME_OVER_1));

    y += GAME_FONT_ASCENT + 1;
    draw_string(MESSAGE_X + 1, y, F(MESSAGE_GAME_OVER_2));

    // draw score
    y += GAME_FONT_ASCENT + 1;
    draw_string(MESSAGE_X + 1, y, F("Score:"));
    draw_int(MESSAGE_X + 1 + (GAME_FONT_WIDTH * 7) , y, score);
    break;
  }
  case GAME_STATE_LEVEL_CLEAR:
  case GAME_STATE_FINISHED:
  {
    game_dim_t y = MESSAGE_Y + 1;
    draw_string(MESSAGE_X + 1, y, F(MESSAGE_LEVEL_CLEAR));

    // draw score remarks
    y += GAME_FONT_ASCENT + 1;
    if (score >= PERFECT_SCORE)
      draw_string(MESSAGE_X + 1, y, F(MESSAGE_CLEAR_PERFECT));
    else if (score >= PASSING_SCORE)
      draw_string(MESSAGE_X + 1, y, F(MESSAGE_CLEAR_PASSING));

    // draw score
    y += GAME_FONT_ASCENT + 1;
    draw_string(MESSAGE_X + 1, y, F("Score:"));
    draw_int(MESSAGE_X + 1 + (GAME_FONT_WIDTH * 7) , y, score);
    break;
  }
  case GAME_STATE_WELCOME:
  {
    game_dim_t y = MESSAGE_Y + 1;
    draw_string(MESSAGE_X + 1, y, F(MESSAGE_WELCOME_1));

    y += GAME_FONT_ASCENT + 1;
    draw_string(MESSAGE_X + 1, y, F(MESSAGE_WELCOME_2));

    y += GAME_FONT_ASCENT + 1;
    draw_string(MESSAGE_X + 1, y, F(MESSAGE_WELCOME_3));
    break;
  }
  }
}

#endif // MARLIN_PRINTIT
