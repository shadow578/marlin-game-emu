#include <iostream> // TODO testing

#if 1

#include "../game.h"


// renderer parameters
constexpr fixed_t PLAYER_FOV = FTOF(3.14159f / 3.0f);
constexpr fixed_t FAR_CLIPPING_PLANE = FTOF(32.0f);
constexpr fixed_t RENDER_STEP_SIZE = FTOF(0.1f);
constexpr fixed_t RENDER_WIDTH = BTOF(GAME_WIDTH);
constexpr fixed_t RENDER_HEIGHT = BTOF(GAME_HEIGHT);

// player controls
constexpr fixed_t PLAYER_STEP_SIZE = FTOF(0.1f);
constexpr fixed_t PLAYER_ROTATION_SPEED = FTOF(0.05f);


inline fixed_t sinff(const fixed_t x)
{
  return FTOF(sin(PTOF(x)));
}

inline fixed_t cosff(const fixed_t x)
{
  return FTOF(cos(PTOF(x)));
}

inline fixed_t deg_to_rad(const fixed_t deg)
{
  return FTOF(PTOF(deg) * (3.14159f / 180.0f));
}


#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_RUNNING 1

#define STATE (marlin_game_data.maze)

void MazeGame::enter_game()
{
  init_game(GAME_STATE_RUNNING, game_screen);

  // reset state
  STATE.world = 0;
  STATE.player.pos = vec2d_t::from(FTOF(2), FTOF(3));
  STATE.player.rotation = FTOF(0);
}

void MazeGame::game_screen()
{
  if (game_state == GAME_STATE_GAME_OVER)
  {
    if (ui.use_click())
      exit_game();
  }
  else if (game_state == GAME_STATE_RUNNING)
  {
    if (ui.use_click())
    {
      const fixed_t rot = deg_to_rad(STATE.player.rotation);
      const vec2d_t dir = vec2d_t::from(sinff(rot), cosff(rot));
      STATE.player.pos = STATE.player.pos + (dir * PLAYER_STEP_SIZE);
    }

    ui.encoderPosition = constrain(ui.encoderPosition, -1, 1);
    STATE.player.rotation += BTOF(ui.encoderPosition) * PLAYER_ROTATION_SPEED;
    ui.encoderPosition = 0;

    std::cout << "pos=" << PTOF(STATE.player.pos.x) << " " << PTOF(STATE.player.pos.y) << " rot=" << PTOF(STATE.player.rotation) << std::endl;
  }

  frame_start();

  set_color(color::WHITE);
  draw_world(get_world(), STATE.player);

  draw_int(0, 0, FTOB(STATE.player.pos.x));
  draw_int(0, GAME_FONT_ASCENT, FTOB(STATE.player.pos.y));
  draw_int(0, GAME_FONT_ASCENT*2, FTOB(STATE.player.rotation));

  frame_end();
}

void MazeGame::draw_world(const world_t *world, const player_t &player)
{
  for(uint8_t x = 0; x < GAME_WIDTH; x++)
  {
    // calculate ray projection angle
    const fixed_t eye_angle = (deg_to_rad(player.rotation) - (PLAYER_FOV / FTOF(2))) + (BTOF(x) / RENDER_WIDTH) * PLAYER_FOV;
    const vec2d_t eye_dir = vec2d_t::from(cosff(eye_angle), sinff(eye_angle));

    // cast a ray from the player until it hits a wall or reaches the far clipping plane
    fixed_t distance = FTOF(0);
    bool hit = false;
    while(distance < FAR_CLIPPING_PLANE)
    {
      distance += RENDER_STEP_SIZE;

      const vec2d_t pos = player.pos + (eye_dir * distance);
      const uint8_t cell_x = FTOB(pos.x);
      const uint8_t cell_y = FTOB(pos.y);

      //std::cout << "ppos=" << PTOF(player.pos.x) << " " << PTOF(player.pos.y) << std::endl;
      //std::cout << "ed=" << PTOF(eye_dir.x) << " " << PTOF(eye_dir.y) << std::endl;
      //std::cout << "d=" << PTOF(distance) << " edist=" << PTOF(eye_distance.x) << " " << PTOF(eye_distance.y) << std::endl;
      //std::cout << "d=" << PTOF(distance) << " px=" << PTOF(pos.x) << " py=" << PTOF(pos.y) << std::endl;
      //std::cout << "cx=" << (int)cell_x << " cy=" << (int)cell_y << std::endl;

      // ray out of bounds?
      if (!world->is_in_bounds(cell_x, cell_y)) break;

      // is a wall?
      if (world->get(cell_x, cell_y))
      {
        hit = true;
        break;
      }
    }

    // had a hit?
    if (!hit) continue;

    // calculate wall points
    const float ceiling = (PTOF(RENDER_HEIGHT) / 2.0f) - (PTOF(RENDER_HEIGHT) / PTOF(distance));
    const float floor = PTOF(RENDER_HEIGHT) - ceiling;

    // draw to the screen
    const uint8_t floor_y = (int)floor;
    const uint8_t ceiling_y = (int)ceiling;

    //std::cout << "d=" << PTOF(distance) << " x=" << (int)x << " cy=" << (int)ceiling_y << " fy=" << (int)floor_y << std::endl;

    //draw_vline(x, ceiling_y, floor_y-ceiling_y);
    draw_pixel(x, ceiling_y);
    draw_pixel(x, floor_y);
  }
}



const MazeGame::world_t* MazeGame::get_world()
{
  return &WORLDS[STATE.world];
}

#endif // MARLIN_DINO
