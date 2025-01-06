#include <iostream> // TODO testing

#if 1

#include "../game.h"

// offset of the boards top left corner from the screen top left corner
constexpr game_dim_t BOARD_OFFSET_X = 2;
constexpr game_dim_t BOARD_OFFSET_Y = 2;

// size of one tetromino block, squared
constexpr game_dim_t TETROMINO_SIZE = 3;

// where on the board the tetromino will spawn, board coordinates
constexpr game_dim_t SPAWN_POINT_X = (TETRIS_BOARD_WIDTH / 2);
constexpr game_dim_t SPAWN_POINT_Y = 0;

// how many milliseconds between each gravity update, ms
// lower causes pieces to fall faster
#define FALL_SPEED(level) (_MAX(50, 250 - (level * 50)))

// location of the next tetromino preview
constexpr game_dim_t NEXT_TETROMINO_X = (BOARD_OFFSET_X + (TETRIS_BOARD_WIDTH * TETROMINO_SIZE) + 3);
constexpr game_dim_t NEXT_TETROMINO_Y = BOARD_OFFSET_Y;

// location of the score display
constexpr game_dim_t SCORE_X = (BOARD_OFFSET_X + (TETRIS_BOARD_WIDTH * TETROMINO_SIZE) + 3);
constexpr game_dim_t SCORE_Y = (NEXT_TETROMINO_Y + (4 * TETROMINO_SIZE) + 2);

#define BOARD_X_TO_SCREEN(x) (BOARD_OFFSET_X + (x * TETROMINO_SIZE))
#define BOARD_Y_TO_SCREEN(y) (BOARD_OFFSET_Y + (y * TETROMINO_SIZE))

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_FALLING_ACTIVE 1
#define GAME_STATE_LINE_CLEAR 2

void TetrisGame::enter_game()
{
  init_game(GAME_STATE_LINE_CLEAR, game_screen);

  // reset state
  marlin_game_data.tetris.board.clear();
  marlin_game_data.tetris.lines_cleared = 0;

  // ensure no falling block is active
  marlin_game_data.tetris.falling.type = tetromino::NONE;
}

void TetrisGame::game_screen()
{
  if (game_state == GAME_STATE_GAME_OVER)
  {
    if (ui.use_click())
      exit_game();
  }
  else if (game_state == GAME_STATE_FALLING_ACTIVE)
  {
    const millis_t now = millis();

    handle_player_input(marlin_game_data.tetris.board, marlin_game_data.tetris.falling);

    if (handle_falling_gravity(marlin_game_data.tetris.board, marlin_game_data.tetris.falling, now, FALL_SPEED(marlin_game_data.tetris.level())))
    {
      // landed on something, commit the falling block
      commit_falling(marlin_game_data.tetris.board, marlin_game_data.tetris.falling);

      // to not continue drawing the falling block, set it to empty
      // not doing this could cause weirdness
      marlin_game_data.tetris.falling.type = tetromino::NONE;

      // check if we need to clear lines
      game_state = GAME_STATE_LINE_CLEAR;
    }
  }
  else if (game_state == GAME_STATE_LINE_CLEAR)
  {
    // clear lines until no more lines to clear
    uint8_t cleared = 0;
    while (handle_line_clear(marlin_game_data.tetris.board))
    {
      cleared++;
    }

    // call handler for lines cleared
    on_lines_cleared(cleared);

    // no more lines to clear, spawn new falling block
    game_state = GAME_STATE_FALLING_ACTIVE;

    if (!spawn_falling(marlin_game_data.tetris.board, marlin_game_data.tetris.falling, marlin_game_data.tetris.next_tetromino))
    {
      // cannot spawn new tetromino, game over
      game_state = GAME_STATE_GAME_OVER;
    }
    else
    {
      // determine next tetromino
      marlin_game_data.tetris.next_tetromino = static_cast<tetromino>(random(0, 7));
    }
  }

  frame_start();
  draw_board(marlin_game_data.tetris.board);

  // draw falling tetromino
  draw_tetromino_shape(BOARD_X_TO_SCREEN(marlin_game_data.tetris.falling.x),
                       BOARD_Y_TO_SCREEN(marlin_game_data.tetris.falling.y),
                       marlin_game_data.tetris.falling.type,
                       marlin_game_data.tetris.falling.rotation);

  // draw next tetromino preview
  set_color(color::WHITE);
  draw_frame(NEXT_TETROMINO_X - 1,
             NEXT_TETROMINO_Y - 1,
             (4 * TETROMINO_SIZE) + 2,
             (4 * TETROMINO_SIZE) + 2);
  draw_tetromino_shape(NEXT_TETROMINO_X,
                       NEXT_TETROMINO_Y,
                       marlin_game_data.tetris.next_tetromino,
                       0);

  // draw score
  set_color(color::WHITE);
  draw_string(SCORE_X, SCORE_Y, "Score:");
  draw_int(SCORE_X, SCORE_Y + GAME_FONT_ASCENT, score);

  // draw level
  draw_string(SCORE_X, SCORE_Y + (2 * GAME_FONT_ASCENT), "Level:");
  draw_int(SCORE_X, SCORE_Y + (3 * GAME_FONT_ASCENT), marlin_game_data.tetris.level());

  if (game_state == GAME_STATE_GAME_OVER)
    draw_game_over();

  frame_end();
}

void TetrisGame::on_falling_committed(const falling_t &falling)
{
  // score based on how many lines the piece fell
  // this is conveniently just the y coordinate of the falling piece
  score += falling.y;
}

void TetrisGame::on_lines_cleared(const uint8_t count)
{
  // score based on how many lines were cleared
  const uint8_t level = marlin_game_data.tetris.level();
  if (count == 1)
    score += level * 100; // Single
  if (count == 2)
    score += level * 300; // Double
  if (count == 3)
    score += level * 500; // Triple
  if (count >= 4)
    score += level * 800; // Tetris

  // update lines cleared in state
  marlin_game_data.tetris.lines_cleared += count;
}

void TetrisGame::handle_player_input(const board_t &board, falling_t &falling)
{
  // record position before update
  falling_t old = falling;
  bool dirty = false;

  // update position when clicking
  if (ui.use_click())
  {
    falling.rotation = (falling.rotation + 1) % 4;
    dirty = true;
  }

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

  if (!dirty)
    return;

  // block movement if it would collide
  if (collision_check_falling(board, falling))
  {
    falling = old;
  }
}

bool TetrisGame::handle_falling_gravity(const board_t &board, falling_t &falling, const millis_t now, const millis_t fall_speed)
{
  // record position before update
  const uint8_t oldY = falling.y;

  // make the block fall
  const bool should_fall = (now - falling.last_update_millis) > fall_speed;
  if (should_fall)
  {
    falling.y++;
    falling.last_update_millis = now;
  }

  // undo falling and commit if collision detected
  if (collision_check_falling(board, falling))
  {
    falling.y = oldY;
    return true;
  }

  return false;
}

bool TetrisGame::handle_line_clear(board_t &board)
{
  // check bottom-to-top for full lines
  for (uint8_t y = TETRIS_BOARD_HEIGHT - 1; y > 0; y--)
  {
    bool full = true;
    for (uint8_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
    {
      if (board.get(x, y) == tetromino::NONE)
      {
        full = false;
        break;
      }
    }

    if (full)
    {
      // clear the line
      // for (uint8_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
      //{
      //  board.set(x, y, tetromino::NONE);
      //}

      // move all lines above one down
      for (uint8_t y2 = y; y2 > 0; y2--)
      {
        for (uint8_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
        {
          board.set(x, y2, board.get(x, y2 - 1));
        }
      }

      // clear the top line
      for (uint8_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
      {
        board.set(x, 0, tetromino::NONE);
      }

      return true;
    }
  }
}

void TetrisGame::commit_falling(board_t &board, const falling_t &falling)
{
  if (falling.type == tetromino::NONE)
  {
    return;
  }

  const uint8_t *shape = get_tetromino_shape(falling.type, falling.rotation);
  for (uint8_t x = 0; x < 4; x++)
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      if (shape[y] & (1 << (3 - x)))
      {
        board.set(falling.x + x, falling.y + y, falling.type);
      }
    }
  }

  on_falling_committed(falling);
}

bool TetrisGame::spawn_falling(const board_t &board, falling_t &falling, const tetromino type)
{
  falling.type = type;
  falling.x = SPAWN_POINT_X;
  falling.y = SPAWN_POINT_Y;
  falling.rotation = 0;

  if (collision_check_falling(board, falling))
  {
    // spawning not possible!
    falling.type = tetromino::NONE;
    return false;
  }

  return true;
}

bool TetrisGame::collision_check_falling(const board_t &board, const falling_t &falling)
{
  if (falling.type == tetromino::NONE)
  {
    // invalid state
    return true;
  }

  const uint8_t *shape = get_tetromino_shape(falling.type, falling.rotation);
  for (uint8_t x = 0; x < 4; x++)
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      if (shape[y] & (1 << (3 - x)))
      {
        const uint8_t collision = board.check_collision(falling.x + x, falling.y + y);
        if (collision != 0)
        {
          std::cout << "collision detected: " << static_cast<int>(collision) << std::endl;
          return true;
        }
      }
    }
  }

  return false;
}

void TetrisGame::draw_board(const board_t &board)
{
  // draw the blocks of the board
  for (uint8_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
  {
    for (uint8_t y = 0; y < TETRIS_BOARD_HEIGHT; y++)
    {
      const tetromino type = board.get(x, y);
      if (type != tetromino::NONE)
      {
        draw_tetromino_block(BOARD_X_TO_SCREEN(x), BOARD_Y_TO_SCREEN(y), type);
      }
    }
  }

  // draw outline of the board
  set_color(color::WHITE);
  draw_frame(BOARD_OFFSET_X - 1,
             BOARD_OFFSET_Y - 1,
             (TETRIS_BOARD_WIDTH * TETROMINO_SIZE) + 2,
             (TETRIS_BOARD_HEIGHT * TETROMINO_SIZE) + 2);
}

void TetrisGame::draw_tetromino_shape(const game_dim_t screen_x, const game_dim_t screen_y, const tetromino type, const uint8_t rotation)
{
  if (type == tetromino::NONE)
  {
    return;
  }

  const uint8_t *shape = get_tetromino_shape(type, rotation);
  for (uint8_t x = 0; x < 4; x++)
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      if (shape[y] & (1 << (3 - x)))
      {
        draw_tetromino_block(screen_x + (x * TETROMINO_SIZE), screen_y + (y * TETROMINO_SIZE), type);
      }
    }
  }
}

void TetrisGame::draw_tetromino_block(const game_dim_t screen_x, const game_dim_t screen_y, const tetromino type)
{
  const size_t c = static_cast<size_t>(type);
  assert(c < sizeof(TETROMINO_COLORS) / sizeof(TETROMINO_COLORS[0]));

  set_color(TETROMINO_COLORS[c]);
  draw_box(screen_x,
           screen_y,
           TETROMINO_SIZE,
           TETROMINO_SIZE);
}

const uint8_t *TetrisGame::get_tetromino_shape(const tetromino type, const uint8_t rotation)
{
  return TETROMINO_SHAPES[static_cast<uint8_t>(type)][rotation];
}



#endif // MARLIN_TETRIS
