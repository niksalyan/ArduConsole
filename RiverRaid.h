#pragma once
#include <Arduino.h>
#include "Engine.h"

class RiverRaid {
public:
  static void begin() {
    shipX = 7; shipY = 13;
    bulletsCount = 0;
    for (int i = 0; i < maxBullets; i++) bulletY[i] = -1;

    riverWidth = 6;
    riverCenter = W / 2;
    frameCounter = 0;
    score = 0;
    waveOffset = 0;

    for (int y = 0; y < H; y++)
      for (int x = 0; x < W; x++)
        terrain[y][x] = 0;

    spawnEnemies();
  }

  static void update() {
    updateInput();
    frameCounter++;
    if (frameCounter >= frameDelay) {
      frameCounter = 0;
      moveRiver();
      moveBullets();
      moveEnemies();
      checkCollisions();
      waveOffset = (waveOffset + 1) % 3; // wave animation
    }
    render();
    Engine::update(50);
  }

private:
  static const int W = 16;
  static const int H = 16;
  inline static const int frameDelay = 1;
  inline static const int maxBullets = 3;
  inline static const int maxEnemies = 5;

  inline static int shipX, shipY;
  inline static int bulletX[maxBullets];
  inline static int bulletY[maxBullets];
  inline static int bulletsCount;
  inline static int enemyX[maxEnemies];
  inline static int enemyY[maxEnemies];
  inline static bool terrain[H][W];

  inline static int riverWidth;
  inline static int riverCenter;

  inline static int score;
  inline static int frameCounter;
  inline static int waveOffset;

  // --- Input ---
  static void updateInput() {
    shipX = constrain(shipX + Engine::getAxisX(), 0, W - 1);
    if (Engine::getKeyDownA()) shootBullet();
  }

  // --- Bullets ---
  static void shootBullet() {
    for (int i = 0; i < maxBullets; i++) {
      if (bulletY[i] < 0) {
        bulletX[i] = shipX;
        bulletY[i] = shipY - 1;
        break;
      }
    }
  }

  static void moveBullets() {
    for (int i = 0; i < maxBullets; i++) {
      if (bulletY[i] >= 0) {
        bulletY[i]--;
        if (bulletY[i] < 0) bulletY[i] = -1;
      }
    }
  }

  // --- Enemies ---
  static void spawnEnemies() {
    for (int i = 0; i < maxEnemies; i++) {
      enemyX[i] = random(0, W);
      enemyY[i] = random(-H, 0);
    }
  }

  static void moveEnemies() {
    for (int i = 0; i < maxEnemies; i++) {
      enemyY[i]++;
      if (enemyY[i] >= H) {
        enemyX[i] = random(0, W);
        enemyY[i] = random(-H / 2, 0);
      }
    }
  }

  // --- River & terrain ---
  static void moveRiver() {
    for (int y = H - 1; y > 0; y--)
      for (int x = 0; x < W; x++)
        terrain[y][x] = terrain[y - 1][x];

    int newCenter = riverCenter + random(-1, 2);
    newCenter = constrain(newCenter, riverWidth / 2, W - 1 - riverWidth / 2);
    riverCenter = newCenter;

    for (int x = 0; x < W; x++)
      terrain[0][x] = (x < riverCenter - riverWidth / 2 || x > riverCenter + riverWidth / 2);
  }

  // --- Collisions ---
  static void checkCollisions() {
    for (int b = 0; b < maxBullets; b++) {
      if (bulletY[b] < 0) continue;
      for (int e = 0; e < maxEnemies; e++) {
        if (bulletX[b] == enemyX[e] && bulletY[b] == enemyY[e]) {
          score++;
          Engine::beep(30, 1200);
          bulletY[b] = -1;
          enemyX[e] = random(0, W);
          enemyY[e] = random(-H / 2, 0);
        }
      }
    }

    for (int e = 0; e < maxEnemies; e++)
      if (enemyX[e] == shipX && enemyY[e] == shipY) {
        begin();
        Engine::beep(100, 800);
      }

    if (terrain[shipY][shipX]) {
      begin();
      Engine::beep(100, 400);
    }
  }

  // --- Rendering ---
  static void render() {
    // --- river & waves ---
    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        if (terrain[y][x]) {
          Engine::setPixel(x, y, Engine::color(0, 100, 0)); // land
        } else {
          // wave animation with blue shades
          uint8_t blue = 50 + (x + y + waveOffset) % 3 * 50;
          Engine::setPixel(x, y, Engine::color(0, 0, blue));
        }
      }
    }

    // bullets
    for (int i = 0; i < maxBullets; i++)
      if (bulletY[i] >= 0) Engine::setPixel(bulletX[i], bulletY[i], Engine::color(255, 255, 0));

    // enemies
    for (int i = 0; i < maxEnemies; i++)
      Engine::setPixel(enemyX[i], enemyY[i], Engine::color(255, 0, 0));

    // plane sprite (white, more like a jet)
    Engine::drawBox(shipX - 1, shipY, 3, 1, Engine::color(255, 255, 255));
    Engine::drawBox(shipX, shipY - 1, 1, 3, Engine::color(255, 255, 255));
    Engine::setPixel(shipX - 1, shipY + 2, Engine::color(64, 64, 64));
    Engine::setPixel(shipX + 1, shipY + 2, Engine::color(64, 64, 64));

    // Engine::setPixel(shipX, shipY + 2, Engine::color(128, 0, 0));

    // score as vertical bars
    for (int i = 0; i < min(score, H); i++)
      Engine::setPixel(W - 1, H - 1 - i, Engine::color(255, 255, 0));
  }
};