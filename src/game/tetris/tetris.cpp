#if 1

#include "../game.h"

// offset of the boards top left corner from the screen top left corner
#define BOARD_OFFSET_X 2
#define BOARD_OFFSET_Y 2

#define TETRAMINO_SIZE 3 // size of one tetramino block, square

void TetrisGame::enter_game()
{
  init_game(1, game_screen);
  marlin_game_data.tetris.board.clear();

  // TODO testing
  marlin_game_data.tetris.board.set(0, 0, Tetromino::I);
  marlin_game_data.tetris.board.set(0, 1, Tetromino::I);
  marlin_game_data.tetris.board.set(0, 2, Tetromino::I);
  marlin_game_data.tetris.board.set(0, 3, Tetromino::I);
}

void TetrisGame::game_screen()
{

  frame_start();
  draw_board(marlin_game_data.tetris.board);
  frame_end();
}

void TetrisGame::draw_board(const board_t &board)
{
  // draw the blocks of the board
  for (size_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
  {
    for (size_t y = 0; y < TETRIS_BOARD_HEIGHT; y++)
    {
      draw_tetromino_block(x, y, board.get(x, y));
    }
  }

  // draw outline of the board
  set_color(color::WHITE);
  draw_frame(BOARD_OFFSET_X - 1,
           BOARD_OFFSET_Y - 1,
           (TETRIS_BOARD_WIDTH * TETRAMINO_SIZE) + 2,
           (TETRIS_BOARD_HEIGHT * TETRAMINO_SIZE) + 2);
}

void TetrisGame::draw_tetromino_block(const size_t board_x, const size_t board_y, const Tetromino type)
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
