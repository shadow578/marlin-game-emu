#pragma once

#include "../types.h"
#include "assert.h"

constexpr size_t TETRIS_BOARD_WIDTH = 10;
constexpr size_t TETRIS_BOARD_HEIGHT = 20;

class TetrisGame : MarlinGame
{
public:
  static void enter_game();
  static void game_screen();

private:
  enum class tetromino : uint8_t
  {
    I = 0,
    J = 1,
    L = 2,
    O = 3,
    S = 4,
    T = 5,
    Z = 6,
    NONE = 7
  };

  struct board_t
  {
    struct
    {
      uint8_t left : 4;
      uint8_t right : 4;
    } board[TETRIS_BOARD_WIDTH / 2][TETRIS_BOARD_HEIGHT];

    void clear()
    {
      for (size_t x = 0; x < TETRIS_BOARD_WIDTH; x++)
      {
        for (size_t y = 0; y < TETRIS_BOARD_HEIGHT; y++)
        {
          set(x, y, tetromino::NONE);
        }
      }
    }

    void set(const uint8_t x, const uint8_t y, const tetromino value)
    {
      assert(x < TETRIS_BOARD_WIDTH);
      assert(y < TETRIS_BOARD_HEIGHT);

      if (x % 2 == 0)
      {
        board[x / 2][y].left = static_cast<uint8_t>(value);
      }
      else
      {
        board[x / 2][y].right = static_cast<uint8_t>(value);
      }
    }

    tetromino get(const uint8_t x, const uint8_t y) const
    {
      assert(x < TETRIS_BOARD_WIDTH);
      assert(y < TETRIS_BOARD_HEIGHT);

      uint8_t value;
      if (x % 2 == 0)
      {
        value = board[x / 2][y].left;
      }
      else
      {
        value = board[x / 2][y].right;
      }

      return static_cast<tetromino>(value);
    }

    /**
     * Check if the given coordinates are within the bounds of the board.
     * @param x The x coordinate.
     * @param y The y coordinate.
     * @return 
     * - 0 - in bounds
     * - 1 - x is out of bounds
     * - 2 - y is out of bounds
     * - 3 - collision with block
     */
    uint8_t check_collision(const uint8_t x, const uint8_t y) const
    {
      if (x >= TETRIS_BOARD_WIDTH)
      {
        return 1;
      }
      if (y >= TETRIS_BOARD_HEIGHT)
      {
        return 2;
      }
      if (get(x, y) != tetromino::NONE)
      {
        return 3;
      }
      return 0;
    }
  };

  struct falling_t {
    tetromino type;
    uint8_t x;
    uint8_t y;

    /**
     * 0 = 0 degrees
     * 1 = 90 degrees
     * 2 = 180 degrees
     * 3 = 270 degrees
     */
    uint8_t rotation;

    millis_t last_update_millis;
  };

public:
  struct state_t
  {
    board_t board;
    falling_t falling;
    tetromino next_tetromino;
  };

private:
  static void handle_player_input(const board_t &board, falling_t &falling);
  static bool handle_falling_gravity(const board_t &board, falling_t &falling, const millis_t now, const millis_t fall_speed);
  static bool handle_line_clear(board_t &board);

  static void commit_falling(board_t &board, const falling_t &falling);
  static bool spawn_falling(const board_t &board, falling_t &falling, const tetromino type);

  static bool collision_check_falling(const board_t &board, const falling_t &falling);
  static const uint8_t* get_falling_shape(const falling_t &falling);

  static void draw_falling(const falling_t &falling);
  static void draw_board(const board_t &board);
  static void draw_tetromino_block(const uint8_t board_x, const uint8_t board_y, const tetromino type);
};

extern TetrisGame tetris;
