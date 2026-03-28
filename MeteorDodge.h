#pragma once
#include <Arduino.h>
#include "Engine.h"

class MeteorDodge {
public:

  static void begin() {
    resetGame();
  }

  static void update() {
    updateInput();
    updateMeteors();
    updateBullets();
    checkCollision();
    bgOffset = (bgOffset + 1) % 32;
    render();
    Engine::update(frameDelay);
  }

private:

  static const uint8_t W = 16;
  static const uint8_t H = 16;
  static const uint8_t frameDelay = 40;
  // ---------------- BULLETS ----------------
  static const uint8_t MAX_BULLETS = 3;

  inline static int8_t bulletX[MAX_BULLETS];
  inline static int8_t bulletY[MAX_BULLETS];

  inline static uint8_t bgOffset = 0;


  // ---------------- PLAYER ----------------
  inline static int8_t playerX;
  static const uint8_t playerY = H - 1;

  // ---------------- METEORS ----------------
  static const uint8_t MAX_METEORS = 6;

  inline static int8_t meteorX[MAX_METEORS];
  inline static int8_t meteorY[MAX_METEORS];

  // ---------------- GAME ----------------
  inline static uint16_t score = 0;
  inline static uint8_t spawnTimer = 0;
  inline static bool gameOver = false;

  // =====================================================
  // RESET
  // =====================================================

  static void resetGame() {

    playerX = W / 2;

    for (uint8_t i = 0; i < MAX_METEORS; i++) {
      meteorY[i] = -1;  // inactive
    }

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
      bulletY[i] = -1;
    }

    score = 0;
    spawnTimer = 0;
    gameOver = false;
  }

  // =====================================================
  // INPUT
  // =====================================================

  static void updateInput() {

    if (gameOver) {
      if (Engine::getKeyDownA()) {
        resetGame();
      }
      return;
    }

    playerX += Engine::getAxisX();

    if (!gameOver && Engine::getKeyDownA()) {
      shoot();
    }

    if (playerX < 0) playerX = 0;
    if (playerX > W - 1) playerX = W - 1;
  }

  // =====================================================
  // METEORS
  // =====================================================

  static void spawnMeteor() {

    for (uint8_t i = 0; i < MAX_METEORS; i++) {

      if (meteorY[i] < 0) {

        meteorX[i] = random(0, W);
        meteorY[i] = 0;
        return;
      }
    }
  }

  static void updateMeteors() {

    if (gameOver) return;

    spawnTimer++;

    if (spawnTimer > 10) {
      spawnTimer = 0;
      spawnMeteor();
    }

    for (uint8_t i = 0; i < MAX_METEORS; i++) {

      if (meteorY[i] >= 0) {

        meteorY[i]++;

        if (meteorY[i] >= H) {
          meteorY[i] = -1;
          score++;
        }
      }
    }
  }

  static void updateBullets() {

    if (gameOver) return;

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {

      if (bulletY[i] >= 0) {

        bulletY[i]--;

        if (bulletY[i] < 0) {
          bulletY[i] = -1;
        }
      }
    }
  }

  static void shoot() {

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {

      if (bulletY[i] < 0) {

        bulletX[i] = playerX;
        bulletY[i] = playerY - 1;

        Engine::beep(10, 1200);
        return;
      }
    }
  }

  // =====================================================
  // COLLISION
  // =====================================================

  static void checkCollision() {

    if (gameOver) return;

    for (uint8_t i = 0; i < MAX_METEORS; i++) {

      if (meteorY[i] == playerY && meteorX[i] == playerX) {

        gameOver = true;
        Engine::beep(200, 200);
      }
    }

    // bullet vs meteor
    for (uint8_t b = 0; b < MAX_BULLETS; b++) {

      if (bulletY[b] < 0) continue;

      for (uint8_t m = 0; m < MAX_METEORS; m++) {

        if (meteorY[m] < 0) continue;

        if (bulletX[b] == meteorX[m] && bulletY[b] == meteorY[m]) {

          bulletY[b] = -1;
          meteorY[m] = -1;

          score += 2;

          Engine::beep(15, 1500);
        }
      }
    }
  }

  // =====================================================
  // RENDER
  // =====================================================

  static void render() {

    // background stars (cheap illusion ✨)
    Engine::drawStars(
	  0, -16 + bgOffset / 2,   // 👈 vertical scroll trick
	  W, H + 16,      // 👈 extend height so no gaps
	  30,
	  Engine::color(0,0,10),
	  Engine::color(0,0,40)
	);

    // player
    Engine::setPixel(playerX, playerY, Engine::color(0, 255, 0));

    // meteors
    for (uint8_t i = 0; i < MAX_METEORS; i++) {

      if (meteorY[i] >= 0) {
        Engine::setPixel(
          meteorX[i],
          meteorY[i],
          Engine::color(255, 50, 0));
      }
    }

    // bullets
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {

      if (bulletY[i] >= 0) {
        Engine::setPixel(
          bulletX[i],
          bulletY[i],
          Engine::color(0, 255, 255));  // cyan laser
      }
    }

    // score
    Engine::drawNumber3x4(0, 0, score, Engine::color(0, 0, 100));

    // game over screen
    if (gameOver) {

      Engine::drawText3x4(2, 6, "END", Engine::color(255, 0, 0));
      Engine::drawText3x4(1, 11, "RST", Engine::color(0, 255, 0));
    }
  }
};