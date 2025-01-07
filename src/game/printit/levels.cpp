#include "../game.h"

const PrintItGame::level_t PrintItGame::levels[PRINTIT_LEVEL_COUNT] = {
  {
    .name = "cube.gcode",
    .init = [](bed_t &bed) {
      const uint16_t shape[] = {
        0b0001111111111000,
        0b0001011111101000,
        0b0001101111011000,
        0b0001110110111000,
        0b0001111001111000,
        0b0001111001111000,
        0b0001110110111000,
        0b0001101111011000,
        0b0001011111101000,
        0b0001111111111000,
      };

      apply_bed_shape(bed, shape, COUNT(shape));
    }
  },
  {
    .name = "boaty.gcode",
    .init = [](bed_t &bed) {
      const uint16_t shape[] = {
        0b0000001100000000,
        0b0000000100011100,
        0b0000111111111000,
        0b0000011100011000,
        0b0000001000001000,
        0b0000001000001000,
        0b0000001000001001,
        0b0000001000001111,
        0b0001111100011011,
        0b1111111101111110,
        0b0111111111111110,
        0b0111111111111100,
        0b0011111111111000,
        0b0001111111110000,
      };

      apply_bed_shape(bed, shape, COUNT(shape));
    },
  },
  {
    .name = "cat.gcode",
    .init = [](bed_t &bed) {
      const uint16_t shape[] = {
        0b0000100000000100,
        0b0000110000001100,
        0b0000111111111100,
        0b0000110111101100,
        0b0000110111101100,
        0b0000111111111100,
        0b1100111100111100,
        0b1100011111111000,
        0b1110001111110000,
        0b0111011111111000,
        0b0011111111111100,
        0b0001111111111100,
        0b0000111111111100,
        0b0000011100111000,
      };

      apply_bed_shape(bed, shape, COUNT(shape));
    },
  },
};

void PrintItGame::apply_bed_shape(bed_t &bed, const uint16_t *shape, const uint8_t shape_height)
{
  for (uint8_t y = 0; y < shape_height; y++)
  {
    for (uint8_t x = 0; x < PRINTIT_BED_WIDTH; x++)
    {
      if (shape[y] & (1 << x))
      {
        bed.set(PRINTIT_BED_WIDTH - x - 1, shape_height - y - 1);
      }
    }
  }
}
