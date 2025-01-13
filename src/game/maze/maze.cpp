#include <iostream> // TODO testing

#if 1

#include "../game.h"


// renderer parameters
constexpr float PI = 3.14159f;
constexpr float PLAYER_FOV = PI / 3.0f;
constexpr float FAR_CLIPPING_PLANE = 32.0f;
constexpr float CELL_CORNER_EPSILON = 0.075f;
constexpr float RENDER_STEP_SIZE = 0.1f;
constexpr float RENDER_WIDTH = GAME_WIDTH;
constexpr float RENDER_HEIGHT = GAME_HEIGHT;

// player controls
constexpr float PLAYER_STEP_SIZE = 0.1f;
constexpr float PLAYER_ROTATION_SPEED = 0.05f;

inline bool equals_approx(const float n, const float m, const float epsilon)
{
  return fabs(n - m) < epsilon;
}

#define GAME_STATE_GAME_OVER 0
#define GAME_STATE_RUNNING 1

#define STATE (marlin_game_data.maze)

void MazeGame::enter_game()
{
  init_game(GAME_STATE_RUNNING, game_screen);

  // reset state
  STATE.world = 0;
  STATE.player.pos = vec2d_t::from(2, 3);
  STATE.player.rotation = 0;
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
      const vec2d_t dir = vec2d_t::from(sinf(STATE.player.rotation), cosf(STATE.player.rotation));
      STATE.player.pos = STATE.player.pos + (dir * PLAYER_STEP_SIZE);
    }

    ui.encoderPosition = constrain(ui.encoderPosition, -1, 1);
    STATE.player.rotation += ui.encoderPosition * PLAYER_ROTATION_SPEED;
    ui.encoderPosition = 0;

    while (STATE.player.rotation > 2 * PI)
      STATE.player.rotation -= 2 * PI;
    while (STATE.player.rotation < 0)
      STATE.player.rotation += 2 * PI;
  
    draw_world_to_console(get_world(), STATE.player);
  }

  frame_start();

  set_color(color::WHITE);
  draw_world(get_world(), STATE.player);

  draw_int(0, 0, STATE.player.pos.x);
  draw_int(0, GAME_FONT_ASCENT, STATE.player.pos.y);
  draw_int(0, GAME_FONT_ASCENT*2, STATE.player.rotation);

  frame_end();
}

void MazeGame::draw_world(const world_t *world, const player_t &player)
{
  for(uint8_t x = 0; x < GAME_WIDTH; x++)
  {
    // calculate ray projection angle
    const float eye_angle = (player.rotation - (PLAYER_FOV / 2.0f)) + (static_cast<float>(x) / RENDER_WIDTH) * PLAYER_FOV;
    const vec2d_t eye_dir = vec2d_t::from(sinf(eye_angle), cosf(eye_angle));

    // cast a ray from the player until it hits a wall or reaches the far clipping plane
    float distance = 0.0f;
    bool hit = false;
    bool is_cell_corner = false;
    while(distance < FAR_CLIPPING_PLANE)
    {
      distance += RENDER_STEP_SIZE;

      const vec2d_t pos = player.pos + (eye_dir * distance);
      const uint8_t cell_x = pos.x;
      const uint8_t cell_y = pos.y;
      
      // ray out of bounds?
      if (!world->is_in_bounds(cell_x, cell_y)) break;

      // is a wall?
      if (world->get(cell_x, cell_y))
      {
        hit = true;

        // calculate texture coordinates
        const vec2d_t mid = vec2d_t::from(cell_x + 0.5f, cell_y + 0.5f);
        const float a = atan2f(pos.y - mid.y, pos.x - mid.x);

        float sx = 0.0f;
        if (a >= -PI * 0.25f && a < PI * 0.25f) sx = pos.y - cell_y;
        else if (a >= PI * 0.25f && a < PI * 0.75f) sx = pos.x - cell_x;
        else if (a >= PI * 0.75f || a < -PI * 0.75f) sx = pos.y - cell_y;
        else if (a >= -PI * 0.75f && a < -PI * 0.25f) sx = pos.x - cell_x;

        // is the corner of a cell?
        is_cell_corner = equals_approx(
          sx,
          0,
          CELL_CORNER_EPSILON
        ) || equals_approx(
          sx,
          1,
          CELL_CORNER_EPSILON
        );

        break;
      }
    }

    // had a hit?
    if (!hit) continue;

    // calculate wall start and end points
    const game_dim_t ceiling = _MAX(0, (RENDER_HEIGHT / 2.0f) - (RENDER_HEIGHT / distance));
    const game_dim_t floor = static_cast<game_dim_t>(RENDER_HEIGHT) - ceiling;

    if (is_cell_corner)
    {
      const game_dim_t h = _MIN(floor - ceiling, static_cast<game_dim_t>(RENDER_HEIGHT) - ceiling);
      draw_vline(x, ceiling, h);
    }
    else
    {
      draw_pixel(x, ceiling);
      draw_pixel(x, floor);
    }
  }
}

void MazeGame::draw_world_to_console(const world_t *world, const player_t &player)
{

  for (int x = 0; x < world->width; x++)
  {
    for (int y = 0; y < world->height; y++)
    {
      if (x == (int)player.pos.x && y == (int)player.pos.y)
      {
        const char* c[] = { "↘", "↓", "↙", "←", "↖", "↑", "↗", "→" };
        int rot = static_cast<int>((player.rotation / (2 * PI)) * COUNT(c)) % COUNT(c);
        std::cout << " " << c[rot] << " ";
      }
      else if (world->get(x, y))
      {
        std::cout << "███";
      }
      else
      {
        std::cout << "   ";
      }
    }

    std::cout << '\n';
  }

  std::cout << "pos=" << STATE.player.pos.x << " " << STATE.player.pos.y << " rot=" << STATE.player.rotation << std::endl;
}

const MazeGame::world_t* MazeGame::get_world()
{
  return &WORLDS[STATE.world];
}

#endif // MARLIN_DINO
