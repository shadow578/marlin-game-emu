#include "../game.h"

const PrintItGame::level_t PrintItGame::levels[PRINTIT_LEVEL_COUNT] = {
  {
    .init = [](bed_t &bed) {
      bed.set(2, 0);
      bed.set(1, 0);
    },
  },
  {
    .init = [](bed_t &bed) {
      bed.set(2, 0);
    }
  }
};
