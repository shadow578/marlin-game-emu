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
  enum class Tetromino : uint8_t
  {
    I = 0,
    J = 1,
    L = 2,
    O = 3,
    S = 4,
    T = 5,
    Z = 6,
    EMPTY = 7
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
          set(x, y, Tetromino::EMPTY);
        }
      }
    }

    void set(const size_t x, const size_t y, const Tetromino value)
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

    Tetromino get(const size_t x, const size_t y) const
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

      return static_cast<Tetromino>(value);
    }
  };

public:
  struct state_t
  {
    board_t board;
  };

private:
  static void draw_board(const board_t &board);
  static void draw_tetromino_block(const size_t board_x, const size_t board_y, const Tetromino type);
};

extern TetrisGame tetris;
