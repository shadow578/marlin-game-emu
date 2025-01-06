#include <iostream> // TODO testing

#if 1

#include "../game.h"

// offset of the boards top left corner from the screen top left corner
#define BOARD_OFFSET_X 2
#define BOARD_OFFSET_Y 2

// size of one tetramino block, squared
#define TETRAMINO_SIZE 3

// how many milliseconds between each gravity update, ms
#define FALL_SPEED 100

// tetramino shapes, each one is a bitmap of 4 rows x 4 bits (LSB)
// top-left corner is the x/y position recorded
const uint8_t TETRAMINO_SHAPE[/*id*/ 7][/*rotation*/ 4][/*row*/ 4] = {
    // I
    {
        // 0 degrees
        {
            0b1000,
            0b1000,
            0b1000,
            0b1000},
        // 90 degrees
        {
            0b1111,
            0b0000,
            0b0000,
            0b0000},
        // 180 degrees
        {
            0b1000,
            0b1000,
            0b1000,
            0b1000},
        // 270 degrees
        {
            0b1111,
            0b0000,
            0b0000,
            0b0000}}

    // TODO other shapes

};

void TetrisGame::enter_game()
{
  init_game(1, game_screen);
  marlin_game_data.tetris.board.clear();

  // TODO testing
  marlin_game_data.tetris.falling.type = Tetromino::I;
  marlin_game_data.tetris.falling.x = 2;
  marlin_game_data.tetris.falling.y = 2;
  marlin_game_data.tetris.falling.rotation = 0;

  marlin_game_data.tetris.board.set(2, 1, Tetromino::Z);

  std::cout << "enter game" << std::endl;
}

void TetrisGame::game_screen()
{
  if (game_state == 1)
  {
    // playing state
    const millis_t now = millis();

    handle_player_input(marlin_game_data.tetris.board, marlin_game_data.tetris.falling);

    if (handle_falling_gravity(marlin_game_data.tetris.board, marlin_game_data.tetris.falling, now, FALL_SPEED))
    {
      commit_falling(marlin_game_data.tetris.board, marlin_game_data.tetris.falling);

      if (!spawn_falling(marlin_game_data.tetris.board, marlin_game_data.tetris.falling))
      {
        // game over
        game_state = 0;
      }
    }
  }
  else if (game_state == 0)
  {
    // game-over state
    if (ui.use_click())
      exit_game();
  }

  frame_start();
  draw_board(marlin_game_data.tetris.board);
  draw_falling(marlin_game_data.tetris.falling);

  if (!game_state)
    draw_game_over();
  frame_end();
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

void TetrisGame::commit_falling(board_t &board, const falling_t &falling)
{
  if (falling.type == Tetromino::EMPTY)
  {
    return;
  }

  const uint8_t *shape = get_falling_shape(falling);
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

  std::cout << "commit falling type=" << static_cast<int>(falling.type) << " x=" << static_cast<int>(falling.x) << " y=" << static_cast<int>(falling.y) << std::endl;
}

bool TetrisGame::spawn_falling(const board_t &board, falling_t &falling)
{
  // TODO: randomize type, update x and y spawn position
  falling.type = Tetromino::I;
  falling.x = 2;
  falling.y = 2;
  falling.rotation = 0;

  if (collision_check_falling(board, falling))
  {
    // spawning not possible!
    falling.type = Tetromino::EMPTY;
    return false;
  }

  return true;
}

bool TetrisGame::collision_check_falling(const board_t &board, const falling_t &falling)
{
  if (falling.type == Tetromino::EMPTY)
  {
    // invalid state
    return true;
  }

  const uint8_t *shape = get_falling_shape(falling);
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

const uint8_t *TetrisGame::get_falling_shape(const falling_t &falling)
{
  return TETRAMINO_SHAPE[static_cast<uint8_t>(falling.type)][falling.rotation];
}

void TetrisGame::draw_falling(const falling_t &falling)
{
  if (falling.type == Tetromino::EMPTY)
  {
    return;
  }

  const uint8_t *shape = get_falling_shape(falling);
  for (uint8_t x = 0; x < 4; x++)
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      if (shape[y] & (1 << (3 - x)))
      {
        draw_tetromino_block(falling.x + x, falling.y + y, falling.type);
      }
    }
  }
}

void TetrisGame::draw_board(const board_t &board)
{
  // draw the blocks of the board
  for (uint8_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
  {
    for (uint8_t y = 0; y < TETRIS_BOARD_HEIGHT; y++)
    {
      const Tetromino type = board.get(x, y);
      if (type != Tetromino::EMPTY)
      {
        draw_tetromino_block(x, y, type);
      }
    }
  }

  // draw outline of the board
  set_color(color::WHITE);
  draw_frame(BOARD_OFFSET_X - 1,
             BOARD_OFFSET_Y - 1,
             (TETRIS_BOARD_WIDTH * TETRAMINO_SIZE) + 2,
             (TETRIS_BOARD_HEIGHT * TETRAMINO_SIZE) + 2);
}

void TetrisGame::draw_tetromino_block(const uint8_t board_x, const uint8_t board_y, const Tetromino type)
{
  const color TETROMINO_COLORS[] = {
      color::RED,     // I
      color::BLUE,    // J
      color::WHITE,   // L
      color::YELLOW,  // O
      color::GREEN,   // S
      color::MAGENTA, // T
      color::CYAN,    // Z
      color::BLACK    // EMPTY
  };

  const size_t c = static_cast<size_t>(type);
  assert(c < sizeof(TETROMINO_COLORS) / sizeof(TETROMINO_COLORS[0]));

  set_color(TETROMINO_COLORS[c]);
  draw_box(BOARD_OFFSET_X + (board_x * TETRAMINO_SIZE),
           BOARD_OFFSET_Y + (board_y * TETRAMINO_SIZE),
           TETRAMINO_SIZE,
           TETRAMINO_SIZE);
}

#endif // MARLIN_TETRIS
