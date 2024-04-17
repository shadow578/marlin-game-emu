#if 1

#include "game.h"
using namespace tennis_game;
static state_t &state = marlin_game_data.tennis;

#define TRANSLATE_X(x) (x)
#define TRANSLATE_Y(y) (GAME_HEIGHT - y)

// Board:
//
//     | player_score_x
// |<------>|      | opponent_score_x
// |<----------------->|  
// |-------------------------------|
// |         00    :    00         |
// | |             :               |
// | |        o    :               |
// |               :               |
// |               :             | |
// |               :             | |
// |               :               |
// |-------------------------------|
// |<------------->|
//         | net_x
constexpr game_dim_t net_x = GAME_WIDTH / 2;
constexpr game_dim_t net_width = 1;
constexpr game_dim_t net_line_length = 4;
constexpr game_dim_t net_line_distance = net_line_length / 2;

constexpr game_dim_t scores_y = GAME_FONT_ASCENT + 1;
constexpr game_dim_t player_score_x = net_x - 2 - (GAME_FONT_WIDTH * 2);
constexpr game_dim_t opponent_score_x = net_x + 2;


// Paddle:
//
//    | paddle_width
//   |-| 
// |------//--
// |  _  ______
// | | |      |
// | | |      | paddle_height
// | | |      |
// | |_| _____|
// ~ 
// |------//--
// |_|
//  | paddle_wall_distance
//
constexpr game_dim_t paddle_width = 2;
constexpr game_dim_t paddle_height = GAME_HEIGHT / 8;
constexpr game_dim_t paddle_wall_distance = 2;

constexpr int8_t paddle_velocity = 3;

constexpr game_dim_t player_paddle_x = paddle_wall_distance;
constexpr game_dim_t opponent_paddle_x = GAME_WIDTH - paddle_wall_distance - paddle_width;


// Ball:
//
//          | ball_size
//        |<->|
// |-//-----------//-|
// ~       ___  ____ ~ _
// |      |   |      | | <- ball_size
// |      |___| ____ | _
// ~                 ~
// |-//-----------//-|
constexpr game_dim_t ball_size = 2;


void TennisGame::enter_game() {
  init_game(1, game_screen);
  reset();
}

void TennisGame::game_screen() {
  // run game logic one per screen
  if (game_frame()) do {
    update_player();
    update_opponent();
    const uint8_t ball_outcome = update_ball();

    // did the ball go out?
    if (ball_outcome != 0) {
      on_score(ball_outcome == 1);
    }
  } while(0);


  // draw the game
  frame_start();

  draw_play_area();
  draw_paddles();
  if (game_state) {
    draw_ball();
  }

  // draw game over screen and exit on click
  if (!game_state) {
    draw_game_over();
    if (ui.use_click()) exit_game();
  }

  frame_end();
}


void TennisGame::reset() {
  score = 0;
  state.opponent_score = 0;
  state.player.y = (GAME_HEIGHT / 2) - (paddle_height / 2);
  state.opponent.y = state.player.y;
  reset_ball();
}

void TennisGame::reset_ball() {
  state.ball.x = BTOF(GAME_WIDTH / 2);
  state.ball.y = BTOF(GAME_HEIGHT / 2);
  state.ball.x_velocity = FTOF(random(-0.5f, 0.5f));
  state.ball.y_velocity = FTOF(random(-0.5f, 0.5f));
}

void TennisGame::update_player() {
  state.player.y += constrain(static_cast<int8_t>(ui.encoderPosition), 0, (GAME_HEIGHT - paddle_height)) / paddle_velocity;
  ui.encoderPosition = state.player.y;
  state.player.y *= paddle_velocity;
}

void TennisGame::update_opponent() {
  // opponent follows the ball
  if (state.ball.y < state.opponent.y) {
    state.opponent.y -= paddle_velocity;
  } else if (state.ball.y > state.opponent.y) {
    state.opponent.y += paddle_velocity;
  }

  state.opponent.y = constrain(state.opponent.y, 0, GAME_HEIGHT - paddle_height);
}

uint8_t TennisGame::update_ball() {
  // provisionally calculate future position to check collisions
  const fixed_t new_x = state.ball.x + state.ball.x_velocity;
  const fixed_t new_y = state.ball.y + state.ball.y_velocity;

  // is y out of the play area?
  if (new_y < 0 || new_y > BTOF(GAME_HEIGHT)) {
    // yes, bounce off boards
    state.ball.y_velocity = -state.ball.y_velocity;
    on_bounce();
  }

  // is x at the player's paddle?
  if (new_x >= BTOF(player_paddle_x) && new_x <= BTOF(player_paddle_x + paddle_width)) {
    // yes, but is y at the paddle?
    if (new_y >= state.player.y && new_y <= state.player.y + BTOF(paddle_height)) {
      // yes, bounce off paddle
      state.ball.x_velocity = -state.ball.x_velocity;
      on_bounce();
    }
  }

  // is x at the opponent's paddle?
  if (new_x >= BTOF(opponent_paddle_x) && new_x <= BTOF(opponent_paddle_x + paddle_width)) {
    // yes, but is y at the paddle?
    if (new_y >= state.opponent.y && new_y <= state.opponent.y + BTOF(paddle_height)) {
      // yes, bounce off paddle
      state.ball.x_velocity = -state.ball.x_velocity;
      on_bounce();
    }
  }

  // is x out of the play area?
  if (new_x < player_paddle_x) {
    // beyond player paddle
    return 1;
  }

  if (new_x > opponent_paddle_x + paddle_width) {
    // beyond opponent paddle
    return 2;
  }

  // not out, actually update positions
  // since the velocity may have changed, we need to re-calculate the new position
  state.ball.x += state.ball.x_velocity;
  state.ball.y += state.ball.y_velocity;
  return 0;
}

void TennisGame::on_score(const bool player) {
  if (player) {
    score++;
  } else {
    state.opponent_score++;
  }

  reset_ball();
}

void TennisGame::on_bounce() {
  _BUZZ(5, 280);
}

void TennisGame::draw_ball() {
  set_color(color::WHITE);

  const game_dim_t ball_x = FTOB(state.ball.x);
  const game_dim_t ball_y = FTOB(state.ball.y);

  if (ball_size > 1) {
    draw_box(
      TRANSLATE_X(ball_x - (ball_size / 2)),
      TRANSLATE_Y(ball_y - (ball_size / 2)),
      ball_size,
      ball_size
    );
  } else {
    draw_pixel(
      TRANSLATE_X(ball_x),
      TRANSLATE_Y(ball_y)
    );
  }
}

void TennisGame::draw_paddles() {
  set_color(color::WHITE);

  // draw player paddle
  draw_box(
    TRANSLATE_X(player_paddle_x),
    TRANSLATE_Y(state.player.y),
    paddle_width,
    paddle_height
  );

  // draw opponent paddle
  draw_box(
    TRANSLATE_X(opponent_paddle_x),
    TRANSLATE_Y(state.opponent.y),
    paddle_width,
    paddle_height
  );
}

void TennisGame::draw_play_area() {
  set_color(color::WHITE);

  // draw net
  for (game_dim_t net_y = 0; net_y < GAME_HEIGHT; net_y += net_line_distance) {
    draw_box(
      TRANSLATE_X(net_x - (net_width / 2)),
      TRANSLATE_Y(net_y),
      net_width,
      net_line_length
    );
  }

  // draw scores
  draw_int(player_score_x, scores_y, score);
  draw_int(opponent_score_x, scores_y, state.opponent_score);
}
#endif // MARLIN_TENNIS
