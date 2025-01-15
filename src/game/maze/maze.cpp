#include <iostream> // TODO testing

#if 1

#include "../game.h"


// renderer parameters
constexpr float PI = 3.14159f;
constexpr float PLAYER_FOV = PI / 3.0f;
constexpr float NEAR_CLIPPING_PLANE = 0.1f;
constexpr float FAR_CLIPPING_PLANE = 32.0f;
constexpr float CELL_CORNER_EPSILON = 0.075f;
constexpr float RENDER_STEP_SIZE = 0.1f;
constexpr float RENDER_WIDTH = GAME_WIDTH;
constexpr float RENDER_HEIGHT = GAME_HEIGHT - 1;

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

  for (auto &entity : STATE.entities)
  {
    entity.type = entity_type::NONE;
  }

  STATE.world = 0;
  load_world(get_world(0), STATE.player);

  // TODO: place a entity in the world
  STATE.entities[0].pos = { 4, 3 };
  STATE.entities[0].type = entity_type::DUMMY;
}

void MazeGame::game_screen()
{
  const world_t *world = get_world(STATE.world);

  if (game_state == GAME_STATE_GAME_OVER)
  {
    if (ui.use_click())
      exit_game();
  }
  else if (game_state == GAME_STATE_RUNNING)
  {
    update_player(world, STATE.player);
    if (check_world_exit(world, STATE.player))
    {
      STATE.world++;
      if (STATE.world >= COUNT(WORLDS))
      {
        STATE.world--;
        game_state = GAME_STATE_GAME_OVER;
      }
      else
      {
        load_world(world, STATE.player);
      }
    }

    update_entities(world, STATE.player, STATE.entities, COUNT(STATE.entities));
  }

  draw_to_console(world, STATE.player, STATE.entities, COUNT(STATE.entities));

  frame_start();

  float depth_buffer[static_cast<size_t>(RENDER_WIDTH)];

  set_color(color::BLUE);
  draw_world(world, STATE.player, depth_buffer);

  set_color(color::RED);
  draw_entities(STATE.entities, COUNT(STATE.entities), STATE.player, depth_buffer);

  draw_int(0, 0, STATE.player.pos.x);
  draw_int(0, GAME_FONT_ASCENT, STATE.player.pos.y);
  draw_int(0, GAME_FONT_ASCENT*2, STATE.player.rotation);

  frame_end();
}

void MazeGame::update_player(const world_t *world, player_t &player)
{
  // move one step forward for each click
  if (ui.use_click())
  {
    const vec2d_t old = player.pos;

    const vec2d_t dir = vec2d_t::from(sinf(player.rotation), cosf(player.rotation));
    player.pos = player.pos + (dir * PLAYER_STEP_SIZE);

    if (world->map.get(player.pos.x, player.pos.y))
    {
      player.pos = old;
    }
  }

  // encoder rotation rotates player
  ui.encoderPosition = constrain(ui.encoderPosition, -1, 1);
  player.rotation += ui.encoderPosition * PLAYER_ROTATION_SPEED;
  ui.encoderPosition = 0;

  // constrain rotation to [0, 2*PI)
  while (player.rotation > 2 * PI)
    player.rotation -= 2 * PI;
  while (player.rotation < 0)
    player.rotation += 2 * PI;
}

void MazeGame::update_entities(const world_t *world, const player_t &player, entity_t *entities, const uint8_t count)
{
  for (uint8_t i = 0; i < count; i++)
  {
    entity_t &entity = entities[i];
    if (entity.type == entity_type::NONE) continue;

    const auto behaviour = get_entity_info(entity.type)->behavior;
    if (behaviour == nullptr) continue;

    behaviour(world, player, entity);
  }
}

void MazeGame::load_world(const world_t *world, player_t &player)
{
  player.pos = world->spawn_point.pos;
  player.rotation = world->spawn_point.rotation;
}

bool MazeGame::check_world_exit(const world_t *world, const player_t &player)
{
  const float dx = player.pos.x - world->exit_point.x;
  const float dy = player.pos.y - world->exit_point.y;

  return (dx * dx + dy * dy < (0.1f));
}

void MazeGame::draw_entities(entity_t *entities, const uint8_t count, const player_t &player, float *depth_buffer)
{
  // sort entities by distance to player
  for (uint8_t i = 0; i < count; ++i) {
    for (uint8_t j = i + 1; j < count; ++j) {
      const float mag_i = (player.pos - entities[i].pos).magnitude_squared();
      const float mag_j = (player.pos - entities[j].pos).magnitude_squared();
      if (mag_i < mag_j) {
        std::swap(entities[i], entities[j]);
      }
    }
  }

  // pre-calculate reusable values for all entities
  const vec2d_t player_view_dir = vec2d_t::from(sinf(player.rotation), cosf(player.rotation));
  const float player_view_angle = atan2f(player_view_dir.y, player_view_dir.x);

  // render each entity
  for (uint8_t i = 0; i < count; ++i) {
    const entity_t& entity = entities[i];

    // is entity active?
    if (entity.type == entity_type::NONE) continue;

    // check if entity is within draw distance
    const vec2d_t to_player = entity.pos - player.pos;
    const float distance = to_player.magnitude();
    if (distance > FAR_CLIPPING_PLANE || distance < NEAR_CLIPPING_PLANE) continue;

    // check if entity is in player's view cone
    const float entity_angle = atan2f(to_player.y, to_player.x);
    float angle = entity_angle - player_view_angle;
    if (angle < -PI) angle += 2.0f * PI;
    if (angle > PI) angle -= 2.0f * PI;

    if (fabs(angle) >= (PLAYER_FOV / 2.0f)) continue;

    // object is visible, let's draw it
    const float ceiling = _MAX(0, (RENDER_HEIGHT / 2.0f) - (RENDER_HEIGHT / distance));
    const float floor = RENDER_HEIGHT - ceiling;

    // determine entity size
    const bitmap_t &sprite = get_entity_info(entity.type)->sprite;
    const float aspect = static_cast<float>(sprite.height) / static_cast<float>(sprite.width);
    const float entity_height = floor - ceiling;
    const float entity_width = entity_height / aspect;

    // determine entity center point
    const float entity_center = (0.5f * (angle / (PLAYER_FOV / 2.0f)) + 0.5f) * RENDER_WIDTH;

    // now draw the entity
    for (float ex = 0; ex < entity_width; ex++)
    {
      for (float ey = 0; ey < entity_height; ey++)
      {
        // determine screen coordinates
        const float rxf = RENDER_WIDTH - (entity_center + ex - (entity_width / 2.0f));
        if (rxf < 0 || rxf >= RENDER_WIDTH) continue;

        const uint8_t rx = static_cast<uint8_t>(rxf);
        const uint8_t ry = ceiling + ey;

        // check if this pixel is occluded
        if (depth_buffer[rx] < distance) continue;

        // sample the entity sprite
        const uint8_t u = static_cast<uint8_t>((ex / entity_width) * sprite.width);
        const uint8_t v = static_cast<uint8_t>((ey / entity_height) * sprite.height);
        if (sprite.get(u, v))
        {
          draw_pixel(rx, ry);
          depth_buffer[rx] = distance;
        }
      }
    }
  }
}

void MazeGame::draw_world(const world_t *world, const player_t &player, float *depth_buffer)
{
  for(uint8_t x = 0; x < RENDER_WIDTH; x++)
  {
    // calculate ray projection angle
    const float eye_angle = (player.rotation - (PLAYER_FOV / 2.0f)) + (static_cast<float>(x) / RENDER_WIDTH) * PLAYER_FOV;
    const vec2d_t eye_dir = vec2d_t::from(sinf(eye_angle), cosf(eye_angle));

    // cast a ray from the player until it hits a wall or reaches the far clipping plane
    float distance = NEAR_CLIPPING_PLANE;
    bool hit = false;
    bool is_cell_corner = false;
    while(distance < FAR_CLIPPING_PLANE)
    {
      distance += RENDER_STEP_SIZE;

      const vec2d_t pos = player.pos + (eye_dir * distance);
      const uint8_t cell_x = pos.x;
      const uint8_t cell_y = pos.y;
      
      // ray out of bounds?
      if (!world->map.is_in_bounds(cell_x, cell_y)) break;

      // is a wall?
      if (world->map.get(cell_x, cell_y))
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

    // record wall in depth buffer
    depth_buffer[x] = distance;

    // calculate wall start and end points
    const game_dim_t ceiling = _MAX(0, (RENDER_HEIGHT / 2.0f) - (RENDER_HEIGHT / distance));
    const game_dim_t floor = static_cast<game_dim_t>(RENDER_HEIGHT) - ceiling;

    if (is_cell_corner)
    {
      draw_vline(x, ceiling, (floor - ceiling) + 1);
    }
    else
    {
      draw_pixel(x, ceiling);
      draw_pixel(x, floor);
    }
  }
}

void MazeGame::draw_to_console(const world_t *world, const player_t &player, const entity_t *entities, const uint8_t entity_count)
{
  for (int x = 0; x < world->map.width; x++)
  {
    for (int y = 0; y < world->map.height; y++)
    {
      const entity_t *entity = nullptr;
      for (uint8_t i = 0; i < entity_count; i++)
      {
        const entity_t *e = &entities[i];
        if (e->type == entity_type::NONE) continue;

        if (x == (int)e->pos.x && y == (int)e->pos.y)
        {
          entity = e;
          break;
        }
      }

      if (x == (int)player.pos.x && y == (int)player.pos.y)
      {
        const char* c[] = { "↘", "↓", "↙", "←", "↖", "↑", "↗", "→" };
        int rot = static_cast<int>((player.rotation / (2 * PI)) * COUNT(c)) % COUNT(c);
        std::cout << " " << c[rot] << " ";
      }
      else if (entity != nullptr)
      {
        std::cout << " " << static_cast<int>(entity->type) << "";
      }
      else if (world->map.get(x, y))
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

#endif // MARLIN_DINO
