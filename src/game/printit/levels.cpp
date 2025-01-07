#include "../game.h"

const PrintItGame::level_t PrintItGame::levels[PRINTIT_LEVEL_COUNT] = {
  {
    .init = [](bed_t &bed) {
      bed.set(2, PRINTIT_BED_HEIGHT - 1);
      bed.set(1, PRINTIT_BED_HEIGHT - 1);
    },
  },
  {
    .init = [](bed_t &bed) {
      bed.set(2, PRINTIT_BED_HEIGHT - 1);
    }
  }
};
