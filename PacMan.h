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

    powerMode = false;
    powerTimer = 0;

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

      // power mode timer
      if (powerMode) {
        if (powerTimer > 0) powerTimer--;
        else powerMode = false;
      }
    }

    render();
    Engine::update(50);
  }

private:

  static const uint8_t W = 16;
  static const uint8_t H = 16;
  static const uint8_t frameDelay = 2;

  static const uint8_t PROGMEM maze[H][W];

  inline static uint16_t dots[H];

  // PACMAN
  inline static int8_t pacX, pacY;
  inline static int8_t dirX, dirY;
  inline static int8_t wantDirX, wantDirY;

  // GHOSTS
  static const uint8_t ghostCount = 4;
  inline static int8_t ghostX[ghostCount];
  inline static int8_t ghostY[ghostCount];
  inline static int8_t ghostDirX[ghostCount];
  inline static int8_t ghostDirY[ghostCount];

  // GAME
  inline static uint16_t score;
  inline static uint8_t frameCounter;

  // POWER MODE
  inline static bool powerMode;
  inline static uint16_t powerTimer;

  // INIT
  static void initDots() {
    for (uint8_t y = 0; y < H; y++) {
      dots[y] = 0;

      for (uint8_t x = 0; x < W; x++) {
        if (pgm_read_byte(&maze[y][x]) == 0) {
          dots[y] |= (1 << x);
        }
      }
    }
  }

  static void initGhosts() {
    for (uint8_t i = 0; i < ghostCount; i++) {
      ghostX[i] = W - 2;
      ghostY[i] = H - 2 - i;

      ghostDirX[i] = -1;
      ghostDirY[i] = 0;
    }
  }

  // INPUT
  static void updateInput() {
    int dx = Engine::getAxisX();
    int dy = Engine::getAxisY();

    if (dx != 0) {
      wantDirX = dx;
      wantDirY = 0;
    } else if (dy != 0) {
      wantDirX = 0;
      wantDirY = dy;
    }
  }

  // PACMAN
  static void movePacMan() {

    int8_t tryX = pacX + wantDirX;
    int8_t tryY = pacY + wantDirY;

    if (isWalkable(tryX, tryY)) {
      dirX = wantDirX;
      dirY = wantDirY;
    }

    int8_t newX = pacX + dirX;
    int8_t newY = pacY + dirY;

    // tunnel wrap
    if (newX < 0) newX = W - 1;
    if (newX >= W) newX = 0;

    if (!isWalkable(newX, newY)) return;

    pacX = newX;
    pacY = newY;

    if (hasDot(pacX, pacY)) {

      bool isPower =
        (pacX == 14 && pacY == 1) ||
        (pacX == 1 && pacY == 14);

      removeDot(pacX, pacY);
      score++;

      if (isPower) {
        powerMode = true;
        powerTimer = 80;
      }

      Engine::beep(10, 1200);
    }
  }

  // GHOSTS
  static void moveGhosts() {

    for (uint8_t g = 0; g < ghostCount; g++) {

      if (random(0, 4) == 0) {
        chooseGhostDirection(g);
        continue;
      }

      int8_t nx = ghostX[g] + ghostDirX[g];
      int8_t ny = ghostY[g] + ghostDirY[g];

      if (!isWalkable(nx, ny)) {
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

    int best = 9999;
    int bestDir = 0;

    for (uint8_t i = 0; i < 4; i++) {

      int8_t nx = ghostX[g] + dirs[i][0];
      int8_t ny = ghostY[g] + dirs[i][1];

      if (!isWalkable(nx, ny)) continue;

      int dist;

      if (powerMode) {
        // run away
        dist = - (abs(nx - pacX) + abs(ny - pacY));
      } else {
        // chase
        dist = abs(nx - pacX) + abs(ny - pacY);
      }

      if (dist < best) {
        best = dist;
        bestDir = i;
      }
    }

    ghostDirX[g] = dirs[bestDir][0];
    ghostDirY[g] = dirs[bestDir][1];

    ghostX[g] += ghostDirX[g];
    ghostY[g] += ghostDirY[g];
  }

  // COLLISIONS
  static void checkCollisions() {

    for (uint8_t g = 0; g < ghostCount; g++) {

      if (pacX == ghostX[g] && pacY == ghostY[g]) {

        if (powerMode) {
          score += 10;

          ghostX[g] = W / 2;
          ghostY[g] = H / 2;
        } else {
          Engine::beep(200, 500);
          delay(400);
          begin();
          return;
        }
      }
    }
  }

  // DOTS
  static bool hasDot(uint8_t x, uint8_t y) {
    return dots[y] & (1 << x);
  }

  static void removeDot(uint8_t x, uint8_t y) {
    dots[y] &= ~(1 << x);
  }

  // HELPERS
  static bool isWalkable(int8_t x, int8_t y) {

    if (x < 0 || x >= W) return true; // tunnels
    if (y < 0 || y >= H) return false;

    return pgm_read_byte(&maze[y][x]) == 0;
  }

  // RENDER
  static void render() {

    bool blink = (millis() / 300) % 2;

    for (uint8_t y = 0; y < H; y++) {
      for (uint8_t x = 0; x < W; x++) {

        if (pgm_read_byte(&maze[y][x]) == 1) {

          Engine::setPixel(x, y, Engine::color(0, 0, 255));

        } else if (hasDot(x,y)) {

          bool isPower =
            (x == 14 && y == 1) ||
            (x == 1 && y == 14);

          if (isPower) {
            if (blink)
              Engine::setPixel(x, y, Engine::color(255,255,255));
          } else {
            Engine::setPixel(x, y, Engine::color(200,200,200));
          }

        } else {
          Engine::setPixel(x, y, Engine::color(0,0,0));
        }
      }
    }

    // pacman
    bool mouth = (millis() / 120) % 2;
    Engine::setPixel(pacX, pacY,
      mouth ? Engine::color(255,255,0)
            : Engine::color(200,200,0));

    // ghost colors
    const uint32_t ghostColors[4] = {
      Engine::color(255,0,0),
      Engine::color(255,105,180),
      Engine::color(0,255,255),
      Engine::color(255,165,0)
    };

    for (uint8_t g = 0; g < ghostCount; g++) {

      if (powerMode) {
        Engine::setPixel(ghostX[g], ghostY[g], Engine::color(0,0,255));
      } else {
        Engine::setPixel(ghostX[g], ghostY[g], ghostColors[g]);
      }
    }

    // score
    //Engine::drawNumber3x4(0, 0, score, Engine::color(255,255,255));
  }
};


// MAZE
const uint8_t PacMan::maze[H][W] PROGMEM = {

{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

{1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},

{1,0,1,1,1,0,1,0,0,1,0,1,1,1,0,1},

{1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},

{1,0,1,0,1,1,1,0,0,1,1,1,0,1,0,1},

{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},

// --- tighter center ---
{1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1},

// --- tunnel row ---
{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},

// --- ghost lane (thin) ---
{1,1,1,0,1,0,0,0,0,0,0,1,0,1,1,1},

{1,0,0,0,1,0,1,1,1,1,0,1,0,0,0,1},

// --- bottom flow ---
{1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1},

{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},

{1,0,1,1,1,0,1,0,0,1,0,1,1,1,0,1},

{1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},

{1,0,1,1,1,0,0,0,0,0,0,1,1,1,0,1},

{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};