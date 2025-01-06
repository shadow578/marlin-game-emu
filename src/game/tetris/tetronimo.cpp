#include "../game.h"

// tetromino shapes, each one is a bitmap of 4 rows x 4 bits (LSB)
// top-left corner is the x/y position recorded
const uint8_t TetrisGame::TETROMINO_SHAPES[/*id*/ 7][/*rotation*/ 4][/*row*/ 4] = {
    // I
    {
        // 0 degrees
        {
            0b1000,
            0b1000,
            0b1000,
            0b1000,
        },
        // 90 degrees
        {
            0b1111,
            0b0000,
            0b0000,
            0b0000,
        },
        // 180 degrees
        {
            0b1000,
            0b1000,
            0b1000,
            0b1000,
        },
        // 270 degrees
        {
            0b1111,
            0b0000,
            0b0000,
            0b0000,
        },
    }

    // TODO other shapes
};

// tetromino colors
const TetrisGame::color TetrisGame::TETROMINO_COLORS[7] = {
    color::RED,     // I
    color::BLUE,    // J
    color::WHITE,   // L
    color::YELLOW,  // O
    color::GREEN,   // S
    color::MAGENTA, // T
    color::CYAN,    // Z
};
