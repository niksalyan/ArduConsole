#pragma once
#include <Arduino.h>
#include <math.h>
#include "Engine.h"

class Arkanoid {
public:
  static void begin() {
    randomSeed(analogRead(A0));
    initLevel();
    resetBall();
    score = 0;
  }

  static void update() {
    updateInput();
    updateBall();
    render();
    Engine::update(frameDelay);
  }

private:
  static const uint8_t W = 14;
  static const uint8_t H = 16;
  static const uint8_t frameDelay = 40;

  // ---------------- GAME ----------------
  inline static uint8_t bricks[6][W];
  inline static uint16_t score;

  // ---------------- PADDLE ----------------
  inline static float paddleX = W / 2.0f - 2.0f;
  static const uint8_t paddleWidth = 4;
  static const uint8_t paddleY = H - 1;

  // ---------------- BALL ----------------
  inline static float ballX, ballY;
  inline static float prevBallX, prevBallY;
  inline static float ballDX, ballDY;
  inline static float ballSpeed = 0.75f;

  // =====================================================
  // LEVEL
  // =====================================================

  static void initLevel() {
    for (uint8_t y = 0; y < 6; y++)
      for (uint8_t x = 0; x < W; x++)
        bricks[y][x] = (y < 4) ? 1 : 0;
  }

  static void resetBall() {
    ballX = W / 2.0f;
    ballY = H - 3.0f;

    float angle = random(-30, 30) * 0.0174533f;
    ballDX = sin(angle);
    ballDY = -fabs(cos(angle));
    normalize();

    ballSpeed = 0.75f;
  }

  static void normalize() {
    float len = sqrt(ballDX * ballDX + ballDY * ballDY);
    if (len == 0) return;
    ballDX /= len;
    ballDY /= len;
  }

  // =====================================================
  // INPUT
  // =====================================================

  static void updateInput() {
    paddleX += Engine::getAxisX() * 0.8f;

    if (paddleX < 0) paddleX = 0;
    if (paddleX > W - paddleWidth)
      paddleX = W - paddleWidth;
  }

  // =====================================================
  // BALL UPDATE
  // =====================================================

  static void updateBall() {

    prevBallX = ballX;
    prevBallY = ballY;

    float nextX = ballX + ballDX * ballSpeed;
    float nextY = ballY + ballDY * ballSpeed;

    // ---------- WALL X ----------
    if (nextX < 0) {
      nextX = 0;
      ballDX *= -1;
      Engine::beep(15, 900);
    }
    if (nextX > W - 1) {
      nextX = W - 1;
      ballDX *= -1;
      Engine::beep(15, 900);
    }

    // ---------- WALL TOP ----------
    if (nextY < 0) {
      nextY = 0;
      ballDY *= -1;
      Engine::beep(15, 900);
    }

    // ---------- PADDLE COLLISION ----------
    // Detect crossing paddle line
    if (ballDY > 0 &&
        prevBallY < paddleY - 0.5f &&
        nextY >= paddleY - 0.5f) {

      if (nextX >= paddleX - 1 &&
          nextX <= paddleX + paddleWidth + 1) {

        float hit = (nextX - paddleX) / paddleWidth;
        float angle = (hit - 0.5f) * 1.4f;

        ballDX = sin(angle);
        ballDY = -fabs(cos(angle));
        normalize();

        nextY = paddleY - 1.0f;

        Engine::beep(40, 1200);
      }
    }

    // ---------- BRICK COLLISION ----------
    int bx = (int)nextX;
    int by = (int)nextY;

    if (by >= 0 && by < 6 &&
        bx >= 0 && bx < W &&
        bricks[by][bx]) {

      bricks[by][bx] = 0;

      // Determine collision side
      if ((int)prevBallY != by)
        ballDY *= -1;
      else
        ballDX *= -1;

      score += 1;
      Engine::beep(60, 1500);

      ballSpeed += 0.015f;
      if (ballSpeed > 1.4f)
        ballSpeed = 1.4f;
    }

    ballX = nextX;
    ballY = nextY;

    // ---------- LOSE ----------
    if (ballY > H - 0.01f) {
      Engine::beep(250, 300);
      initLevel();
      resetBall();
      score = 0;
    }
  }

  // =====================================================
  // RENDER
  // =====================================================

  static void render() {

    // bg
    Engine::drawBricks(1, 0, W, H, Engine::color(0,0,2), CRGB::Black);

    // Engine::drawText3x4(1, 4, "ARCA", Engine::color(0,0,7));
    // Engine::drawText3x4(1, 9, "NOID", Engine::color(0,0,7));

    // Engine::drawNumber3x4Right(14, 11, score, Engine::color(2,2,32));

    // Borders
    for (uint8_t y = 0; y < H; y++) {
      Engine::setPixel(0, y, Engine::color(70, 70, 70));
      Engine::setPixel(W + 1, y, Engine::color(70, 70, 70));
    }

    // Bricks
    for (uint8_t y = 0; y < 6; y++)
      for (uint8_t x = 0; x < W; x++)
        if (bricks[y][x])
          Engine::setPixel(x + 1, y,
            Engine::color(255, 40 + y * 40, 0));

    // Paddle
    for (uint8_t i = 0; i < paddleWidth; i++)
      Engine::setPixel((int)paddleX + i + 1, paddleY, CRGB::Green);

    // Ball
    Engine::setPixel((int)ballX + 1, (int)ballY, CRGB::White);


    

    // Score bars
    uint16_t temp = score;
    uint8_t baseX = W + 4;

    for (uint8_t i = 0; i < 3; i++) {
      uint8_t digit = temp % 10;
      temp /= 10;

      for (uint8_t d = 0; d < digit && d < H; d++)
        Engine::setPixel(baseX + i, H - 1 - d, CRGB::Yellow);
    }
  }
};