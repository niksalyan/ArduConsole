#pragma once
#include <Arduino.h>
#include "Engine.h"

class SpaceInvaders {
public:

  static void begin() {
    initPlayer();
    initEnemies();
    bulletsCount = 0;
    enemyBulletsCount = 0;
  }

  static void update() {

    updatePlayer();
    updateBullets();
    updateEnemyBullets();
    updateEnemies();
    checkCollisions();

    render();
    Engine::update(frameDelay);
  }

private:

  static const uint8_t W = 16;
  static const uint8_t H = 16;

  // ---------- PLAYER ----------

  inline static int8_t playerX;
  inline static const int8_t playerY = H-1;

  // ---------- PLAYER BULLETS ----------

  static const uint8_t maxBullets = 4;

  inline static int8_t bulletsX[maxBullets];
  inline static int8_t bulletsY[maxBullets];
  inline static uint8_t bulletsCount = 0;

  // ---------- ENEMY BULLETS ----------

  static const uint8_t maxEnemyBullets = 3;

  inline static int8_t enemyBulletsX[maxEnemyBullets];
  inline static int8_t enemyBulletsY[maxEnemyBullets];
  inline static uint8_t enemyBulletsCount = 0;

  // ---------- ENEMIES ----------

  static const uint8_t enemyRows = 3;

  // 6 invaders stored as bits
  inline static uint8_t enemyMask[enemyRows];

  inline static int8_t enemyOffsetX = 3;
  inline static int8_t enemyOffsetY = 1;
  inline static int8_t enemyDir = 1;

  inline static uint8_t enemyMoveCounter = 0;

  // ---------- TIMING ----------

  static const uint8_t frameDelay = 70;

  // ---------- INIT ----------

  static void initPlayer() {
    playerX = W/2;
  }

  static void initEnemies() {

    for (uint8_t y=0;y<enemyRows;y++)
      enemyMask[y] = 0b00111110; // 6 enemies

    enemyOffsetX = 3;
    enemyOffsetY = 1;
    enemyDir = 1;
  }

  // ---------- PLAYER ----------

  static void updatePlayer() {

    playerX = constrain(playerX + Engine::getAxisX(),1,W-2);

    if (Engine::getKeyDownA() && bulletsCount < maxBullets) {

      bulletsX[bulletsCount] = playerX;
      bulletsY[bulletsCount] = playerY-1;
      bulletsCount++;

      Engine::beep(20,1100);
    }
  }

  // ---------- BULLETS ----------

  static void updateBullets() {

    for (uint8_t i=0;i<bulletsCount;i++) {

      bulletsY[i]--;

      if (bulletsY[i] < 0) {
        bulletsY[i] = bulletsY[--bulletsCount];
        bulletsX[i] = bulletsX[bulletsCount];
        i--;
      }
    }
  }

  // ---------- ENEMY BULLETS ----------

  static void updateEnemyBullets() {

    for (uint8_t i=0;i<enemyBulletsCount;i++) {

      enemyBulletsY[i]++;

      if (enemyBulletsY[i] >= H) {
        enemyBulletsY[i] = enemyBulletsY[--enemyBulletsCount];
        enemyBulletsX[i] = enemyBulletsX[enemyBulletsCount];
        i--;
      }
    }
  }

  // ---------- ENEMY MOVEMENT ----------

  static void updateEnemies() {

    enemyMoveCounter++;

    if (enemyMoveCounter < 8) return;

    enemyMoveCounter = 0;

    enemyOffsetX += enemyDir;

    if (enemyOffsetX <= 0 || enemyOffsetX + 6 >= W) {

      enemyDir = -enemyDir;
      enemyOffsetY++;

      enemyOffsetX += enemyDir;

      enemyShoot();
    }

    if (enemyOffsetY >= H-3) {
      begin(); // game over
    }
  }

  // ---------- ENEMY SHOOT ----------

  static void enemyShoot() {

    if (enemyBulletsCount >= maxEnemyBullets) return;

    uint8_t col = random(0,6);

    for (int y=enemyRows-1;y>=0;y--) {

      if (enemyMask[y] & (1<<col)) {

        enemyBulletsX[enemyBulletsCount] = enemyOffsetX + col;
        enemyBulletsY[enemyBulletsCount] = enemyOffsetY + y;

        enemyBulletsCount++;
        return;
      }
    }
  }

  // ---------- COLLISIONS ----------

  static void checkCollisions() {

    // player bullets -> enemies
    for (uint8_t b=0;b<bulletsCount;b++) {

      int bx = bulletsX[b] - enemyOffsetX;
      int by = bulletsY[b] - enemyOffsetY;

      if (bx>=0 && bx<6 && by>=0 && by<enemyRows) {

        if (enemyMask[by] & (1<<bx)) {

          enemyMask[by] &= ~(1<<bx);

          bulletsX[b] = bulletsX[--bulletsCount];
          bulletsY[b] = bulletsY[bulletsCount];
          b--;

          Engine::beep(40,1400);
        }
      }
    }

    // enemy bullets -> player
    for (uint8_t i=0;i<enemyBulletsCount;i++) {

      if (enemyBulletsY[i]==playerY &&
          abs(enemyBulletsX[i]-playerX)<=1) {

        Engine::beep(200,400);
        delay(300);
        begin();
        return;
      }
    }
  }

  // ---------- RENDER ----------

  static void render() {

    // PLAYER (3 pixels wide)
    Engine::setPixel(playerX-1,playerY,Engine::color(0,255,0));
    Engine::setPixel(playerX,playerY,Engine::color(0,255,0));
    Engine::setPixel(playerX+1,playerY,Engine::color(0,255,0));

    // PLAYER BULLETS
    for(uint8_t i=0;i<bulletsCount;i++)
      Engine::setPixel(bulletsX[i],bulletsY[i],Engine::color(255,255,0));

    // ENEMY BULLETS
    for(uint8_t i=0;i<enemyBulletsCount;i++)
      Engine::setPixel(enemyBulletsX[i],enemyBulletsY[i],Engine::color(255,0,255));

    // ENEMIES
    for(uint8_t y=0;y<enemyRows;y++) {

      for(uint8_t x=0;x<6;x++) {

        if(enemyMask[y]&(1<<x)) {

          int px = enemyOffsetX + x;
          int py = enemyOffsetY + y;

          Engine::setPixel(px,py,Engine::color(255,0,0));

          if((millis()/200)%2)
            Engine::setPixel(px,py+1,Engine::color(255,60,0));
        }
      }
    }
  }
};