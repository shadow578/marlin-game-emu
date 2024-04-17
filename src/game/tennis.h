#pragma once

#include "types.h"

namespace tennis_game 
{
  typedef struct {
    /**
     * @brief Ball X position 
     */
    fixed_t x;

    /**
     * @brief Ball Y position 
     */
    fixed_t y;

    /**
     * @brief Ball X velocity 
     */
    fixed_t x_velocity;

    /**
     * @brief Ball Y velocity 
     */
    fixed_t y_velocity;
  } ball_state_t;

  typedef struct {
    /**
     * @brief Paddle Y position. X position is fixed.  
     */
    int8_t y;
  } paddle_state_t;

  typedef struct {
    /**
     * @brief Current ball state 
     */
    ball_state_t ball;

    /**
     * @brief Current player paddle state
     */
    paddle_state_t player;

    /**
     * @brief Current opponent paddle state
     */
    paddle_state_t opponent;

    /**
     * @brief Score of the opponent. 
     * Player score is located in the MarlinGame base class.
     */
    int opponent_score;
  } state_t;

  class TennisGame : MarlinGame {
  public:
    static void enter_game(), 
                game_screen();

    private:
    /**
     * @brief Reset game state 
     */
    static void reset();

    /**
     * @brief Reset the ball to the center of the play area and give it a random velocity 
     */
    static void reset_ball();

    /**
     * @brief Handle input from the player, moving the paddle up or down 
     */
    static void update_player();

    /**
     * @brief Run the opponent AI, moving the paddle up or down  
     */
    static void update_opponent();

    /**
     * @brief Move the ball and handle collisions
     * @return 
     * 0 if the ball is still in play, 
     * 1 if the ball is out on the player side, 
     * 2 if the ball is out on the opponent side
     */
    static uint8_t update_ball();

    /**
     * @brief Called when the player or opponent scores
     * @param player true if the player scored, false if the opponent scored 
     */
    static void on_score(const bool player);

    /**
     * @brief Called every time the ball bounces off something 
     */
    static void on_bounce();

    /**
     * @brief Draw the ball 
     */
    static void draw_ball();

    /**
     * @brief Draw both paddles 
     */
    static void draw_paddles();

    /**
     * @brief Draw the playing area (net, score, etc.)
     */
    static void draw_play_area();
  };

  extern TennisGame tennis;
} // namespace tennis_game

