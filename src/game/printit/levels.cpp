#include "../game.h"

const PrintItGame::level_t PrintItGame::levels[PRINTIT_LEVEL_COUNT] = {
  {
    .name = "The Benchy",
    .init = [](bed_t &bed) {
      bed.set(2, 0);
      bed.set(1, 0);
    },
  },
  {
    .name = "The Cube",
    .init = [](bed_t &bed) {
      bed.set(2, 0);
    }
  }
};
