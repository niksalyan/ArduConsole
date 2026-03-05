#pragma once
#include <Arduino.h>
#include "Engine.h"

class PacMan {
public:

  static void begin() {
    randomSeed(analogRead(A0));

    pacX = 1;
    pacY = 1;

    dirX = 1;
    dirY = 0;

    wantDirX = 1;
    wantDirY = 0;

    score = 0;
    frameCounter = 0;

    initDots();
    initGhosts();
  }

  static void update() {

    updateInput();

    frameCounter++;
    if (frameCounter >= frameDelay) {
      frameCounter = 0;

      movePacMan();
      moveGhosts();
      checkCollisions();
    }

    render();
    Engine::update(50);
  }

private:

  static const uint8_t W = 16;
  static const uint8_t H = 16;
  static const uint8_t frameDelay = 2;

  // ---------- MAZE (FLASH) ----------

  static const uint8_t PROGMEM maze[H][W];

  // ---------- DOTS (RAM bitmask) ----------

  inline static uint16_t dots[H];   // 16 rows × 16 bits

  // ---------- PACMAN ----------

  inline static uint8_t pacX;
  inline static uint8_t pacY;

  inline static int8_t dirX;
  inline static int8_t dirY;

  inline static int8_t wantDirX;
  inline static int8_t wantDirY;

  // ---------- GHOSTS ----------

  static const uint8_t ghostCount = 4;

  inline static uint8_t ghostX[ghostCount];
  inline static uint8_t ghostY[ghostCount];

  inline static int8_t ghostDirX[ghostCount];
  inline static int8_t ghostDirY[ghostCount];

  // ---------- GAME ----------

  inline static uint8_t score;
  inline static uint8_t frameCounter;

  // ---------- INIT ----------

  static void initDots() {

    for (uint8_t y = 0; y < H; y++) {
      dots[y] = 0;

      for (uint8_t x = 0; x < W; x++) {
        if (pgm_read_byte(&maze[y][x]) == 0) {
          dots[y] |= (1 << x);
        }
      }
    }

    // power pellets
    dots[1] |= (1 << 14);
    dots[14] |= (1 << 1);
  }

  static void initGhosts() {

    for (uint8_t i = 0; i < ghostCount; i++) {
      ghostX[i] = W - 2;
      ghostY[i] = H - 2 - i;

      ghostDirX[i] = -1;
      ghostDirY[i] = 0;
    }
  }

  // ---------- INPUT ----------

  static void updateInput() {
    int dx = Engine::getAxisX();
    int dy = Engine::getAxisY();

    if (dx != 0) {
      wantDirX = dx;
      wantDirY = 0;
    }
    else if (dy != 0) {
      wantDirX = 0;
      wantDirY = dy;
    }
  }
  // ---------- PACMAN ----------

  static void movePacMan() {

    // try to apply queued direction first
    int8_t tryX = pacX + wantDirX;
    int8_t tryY = pacY + wantDirY;

    if (isWalkable(tryX, tryY)) {
      dirX = wantDirX;
      dirY = wantDirY;
    }

    int8_t newX = pacX + dirX;
    int8_t newY = pacY + dirY;

    if (!isWalkable(newX, newY))
      return;

    pacX = newX;
    pacY = newY;

    if (hasDot(pacX, pacY)) {

      removeDot(pacX, pacY);
      score++;

      Engine::beep(15, 1200);
    }
  }

  // ---------- GHOSTS ----------

  static void moveGhosts() {

    for (uint8_t g = 0; g < ghostCount; g++) {

      int8_t nx = ghostX[g] + ghostDirX[g];
      int8_t ny = ghostY[g] + ghostDirY[g];

      if (!isWalkable(nx, ny) || random(0, 5) == 0) {
        chooseGhostDirection(g);
      } else {
        ghostX[g] = nx;
        ghostY[g] = ny;
      }
    }
  }

  static void chooseGhostDirection(uint8_t g) {

    const int8_t dirs[4][2] = {
      {1,0},{-1,0},{0,1},{0,-1}
    };

    for (uint8_t i = 0; i < 10; i++) {

      uint8_t r = random(0,4);

      int8_t nx = ghostX[g] + dirs[r][0];
      int8_t ny = ghostY[g] + dirs[r][1];

      if (isWalkable(nx, ny)) {

        ghostDirX[g] = dirs[r][0];
        ghostDirY[g] = dirs[r][1];

        ghostX[g] = nx;
        ghostY[g] = ny;
        return;
      }
    }
  }

  // ---------- COLLISIONS ----------

  static void checkCollisions() {

    for (uint8_t g = 0; g < ghostCount; g++) {

      if (pacX == ghostX[g] && pacY == ghostY[g]) {

        Engine::beep(200, 500);
        delay(400);

        begin();
        return;
      }
    }
  }

  // ---------- DOTS ----------

  static bool hasDot(uint8_t x, uint8_t y) {
    return dots[y] & (1 << x);
  }

  static void removeDot(uint8_t x, uint8_t y) {
    dots[y] &= ~(1 << x);
  }

  // ---------- HELPERS ----------

  static bool isWalkable(int8_t x, int8_t y) {

    if (x < 0 || y < 0 || x >= W || y >= H)
      return false;

    return pgm_read_byte(&maze[y][x]) == 0;
  }

  // ---------- RENDER ----------

  static void render() {

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {

        if (pgm_read_byte(&maze[y][x]) == 1) {

          Engine::setPixel(x, y, Engine::color(0,0,180));

        } else if (hasDot(x,y)) {

          Engine::setPixel(x, y, Engine::color(255,120,0));

        } else {

          Engine::setPixel(x, y, Engine::color(0,0,0));
        }
      }
    }

    // pacman
    Engine::setPixel(pacX, pacY, Engine::color(255,255,0));

    // ghosts
    for (uint8_t g = 0; g < ghostCount; g++) {

      Engine::setPixel(
        ghostX[g],
        ghostY[g],
        Engine::color(255,0,0)
      );
    }
  }
};


// ---------- PACMAN MAZE ----------

const uint8_t PacMan::maze[H][W] PROGMEM = {

{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
{1,0,1,1,1,0,1,0,0,1,0,1,1,1,0,1},
{1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
{1,0,1,0,1,1,1,0,0,1,1,1,0,1,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1},
{1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
{1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1},
{1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
{1,1,1,0,1,1,1,0,0,1,1,1,0,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,0,1,0,0,1,0,1,1,1,0,1},
{1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
{1,0,1,1,1,0,0,0,0,0,0,1,1,1,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}

};