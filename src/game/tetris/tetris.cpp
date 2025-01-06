#include <iostream> // TODO testing


#if 1

#include "../game.h"

// offset of the boards top left corner from the screen top left corner
#define BOARD_OFFSET_X 2
#define BOARD_OFFSET_Y 2

// size of one tetramino block, squared
#define TETRAMINO_SIZE 3

// tetramino shapes, each one is a bitmap of 4 rows x 4 bits (LSB)
// top-left corner is the x/y position recorded
const uint8_t TETRAMINO_SHAPE[/*id*/7][/*rotation*/4][/*row*/4] = {
  // I
  {
    // 0 degrees
    {
      0b1000,
      0b1000,
      0b1000,
      0b1000
    },
    // 90 degrees
    {
      0b1111,
      0b0000,
      0b0000,
      0b0000
    },
    // 180 degrees
    {
      0b1000,
      0b1000,
      0b1000,
      0b1000
    },
    // 270 degrees
    {
      0b1111,
      0b0000,
      0b0000,
      0b0000
    }
  }

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

  std::cout << "enter game" << std::endl;
}

void TetrisGame::game_screen()
{
  update_falling(marlin_game_data.tetris.board, marlin_game_data.tetris.falling);

  frame_start();
  draw_board(marlin_game_data.tetris.board);
  draw_falling(marlin_game_data.tetris.falling);
  frame_end();
}

void TetrisGame::update_falling(const board_t board, falling_t &falling)
{
  // record position before update
  falling_t old = falling;
  bool dirty = false;

  // update position when clicking
  if (ui.use_click()) {
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

  if (!dirty) return;

  // block movement if it would go out of bounds
  const uint8_t bounds = bound_check_falling(board, falling);
  if (bounds != 0)
  {
    falling = old;
  }
}

uint8_t TetrisGame::bound_check_falling(const board_t &board, const falling_t &falling)
{
  if (falling.type == Tetromino::EMPTY)
  {
    // only check bounds of origin to avoid invalid state
    return board.check_bounds(falling.x, falling.y);
  }

  const uint8_t *shape = TETRAMINO_SHAPE[static_cast<uint8_t>(falling.type)][falling.rotation];
  for (uint8_t x = 0; x < 4; x++)
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      if (shape[y] & (1 << (4 - x)))
      {
        const uint8_t bounds = board.check_bounds(falling.x + x, falling.y + y);
        if (bounds != 0)
        {
          return bounds;
        }
      }
    }
  }

  return 0;
}

void TetrisGame::draw_falling(const falling_t &falling)
{
  if (falling.type == Tetromino::EMPTY)
  {
    return;
  }

  const uint8_t *shape = TETRAMINO_SHAPE[static_cast<uint8_t>(falling.type)][falling.rotation];
  for (uint8_t x = 0; x < 4; x++)
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      if (shape[y] & (1 << (4 - x)))
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
