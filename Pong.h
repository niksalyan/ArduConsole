#pragma once
#include <Arduino.h>
#include <math.h>
#include "Engine.h"

class Pong {
public:

  static void begin() {
    randomSeed(analogRead(A0));
    resetGame();
  }

  static void update() {
    updateInput();
    updateAI();
    updateBall();
    render();
    Engine::update(frameDelay);
  }

  // --- Score getters (for your UI) ---
  static uint8_t getPlayerScore() { return playerScore; }
  static uint8_t getAIScore()     { return aiScore; }

private:

  static const uint8_t W = 14;
  static const uint8_t H = 16;
  static const uint8_t frameDelay = 35;

  // ---------------- SCORE ----------------
  inline static uint8_t playerScore = 0;
  inline static uint8_t aiScore = 0;

  // ---------------- PADDLES ----------------
  static const uint8_t paddleWidth = 4;

  inline static float playerX;
  inline static float aiX;

  static const uint8_t playerY = H - 1;
  static const uint8_t aiY = 0;

  // ---------------- BALL ----------------
  inline static float ballX, ballY;
  inline static float ballDX, ballDY;
  inline static float ballSpeed;

  // ---------------- AI ----------------
  inline static uint8_t aiReactionTimer = 0;
  inline static float aiTargetX = 0;

  // =====================================================
  // RESET
  // =====================================================

  static void resetGame() {

    playerScore = 0;
    aiScore = 0;

    playerX = W / 2.0f - paddleWidth / 2.0f;
    aiX     = playerX;

    resetBall(random(0,2)==0 ? -1 : 1);
  }

  static void resetBall(int dir) {

    ballX = W / 2.0f;
    ballY = H / 2.0f;

    float angle = random(-40, 40) * 0.0174533f;

    ballDX = sin(angle);
    ballDY = dir * fabs(cos(angle));

    normalize();

    ballSpeed = 0.75f;
  }

  static void normalize() {

    float len = sqrt(ballDX*ballDX + ballDY*ballDY);

    if (len == 0) return;

    ballDX /= len;
    ballDY /= len;
  }

  // =====================================================
  // PLAYER INPUT
  // =====================================================

  static void updateInput() {

    playerX += Engine::getAxisX() * 1.0f;

    if (playerX < 0) playerX = 0;

    if (playerX > W - paddleWidth)
      playerX = W - paddleWidth;
  }

  // =====================================================
  // IMPROVED AI
  // =====================================================

  static void updateAI() {

    aiReactionTimer++;

    // AI only recalculates sometimes (reaction delay)
    if (aiReactionTimer > 6) {

      aiReactionTimer = 0;

      float predictedX = ballX;

      // Only track if ball moving toward AI
      if (ballDY < 0) {

        float steps = (ballY - aiY) / fabs(ballDY);
        predictedX = ballX + ballDX * steps;

        // wall reflections
        while (predictedX < 0 || predictedX > W-1) {

          if (predictedX < 0)
            predictedX = -predictedX;

          if (predictedX > W-1)
            predictedX = 2*(W-1) - predictedX;
        }
      }

      // Add human-like aiming error
      predictedX += random(-2,3);

      aiTargetX = predictedX - paddleWidth/2.0f;
    }

    float diff = aiTargetX - aiX;

    float maxSpeed = 0.45f;   // slower than player

    if (diff > maxSpeed) diff = maxSpeed;
    if (diff < -maxSpeed) diff = -maxSpeed;

    aiX += diff;

    if (aiX < 0) aiX = 0;

    if (aiX > W - paddleWidth)
      aiX = W - paddleWidth;
  }

  // =====================================================
  // BALL PHYSICS
  // =====================================================

  static void updateBall() {

    float nextX = ballX + ballDX * ballSpeed;
    float nextY = ballY + ballDY * ballSpeed;

    // ---- Side walls ----
    if (nextX < 0) {

      nextX = 0;
      ballDX *= -1;

      Engine::beep(10,900);
    }

    if (nextX > W-1) {

      nextX = W-1;
      ballDX *= -1;

      Engine::beep(10,900);
    }

    // ---- AI paddle ----
    if (ballDY < 0 && nextY <= aiY + 0.5f) {

      if (nextX >= aiX-0.5f &&
          nextX <= aiX+paddleWidth+0.5f) {

        bounce(aiX);
        nextY = aiY + 1;

        Engine::beep(25,1200);
      }
    }

    // ---- Player paddle ----
    if (ballDY > 0 && nextY >= playerY - 0.5f) {

      if (nextX >= playerX-0.5f &&
          nextX <= playerX+paddleWidth+0.5f) {

        bounce(playerX);
        nextY = playerY - 1;

        Engine::beep(25,1200);
      }
    }

    ballX = nextX;
    ballY = nextY;

    // ---- Scoring ----
    if (ballY < -0.5f) {

      playerScore++;

      Engine::beep(180,400);
      resetBall(1);
    }

    if (ballY > H-0.5f) {

      aiScore++;

      Engine::beep(180,200);
      resetBall(-1);
    }
  }

  static void bounce(float paddleXRef) {

    float hit = (ballX - paddleXRef) / paddleWidth;

    float angle = (hit - 0.5f) * 1.7f;

    ballDX = sin(angle);
    ballDY *= -1;

    normalize();

    ballSpeed += 0.04f;

    if (ballSpeed > 1.7f)
      ballSpeed = 1.7f;
  }

  // =====================================================
  // RENDER
  // =====================================================

  static void render() {

    // center line
    Engine::drawChecker(W/2, 0, 2, H, Engine::color(0,0,4), Engine::color(0,0,0));

    // score
    Engine::drawNumber3x4Right(6, 2, playerScore, Engine::color(0,0,64));
    Engine::drawNumber3x4(9, 2, aiScore, Engine::color(0,0,64));

    // player paddle
    Engine::drawBox(playerX + 1, playerY, paddleWidth, 1, Engine::color(0,255,0));

    // AI paddle
    Engine::drawBox(aiX + 1, aiY, paddleWidth, 1, Engine::color(255,0,0));

    // ball
    Engine::setPixel((int)ballX+1, (int)ballY,
      Engine::color(255,255,255));
  }
};