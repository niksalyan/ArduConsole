#pragma once
#include <Arduino.h>
#include "Engine.h"

class SpaceInvaders {
public:

  static void begin() {

    score = 0;
    level = 1;
    gameOver = false;

    initPlayer();
    initEnemies();

    bulletsCount = 0;
    enemyBulletsCount = 0;

    ufoX = -1;
  }

  static void update() {

    if(gameOver)
    {
      renderGameOver();
      Engine::update(80);

      if(Engine::getKeyDownA())
        begin();

      return;
    }

    updatePlayer();
    updateBullets();
    updateEnemyBullets();
    updateEnemies();
    updateUFO();

    checkCollisions();

    if(!enemiesAlive())
    {
      level++;
      initEnemies();
      bulletsCount = 0;
      enemyBulletsCount = 0;

      Engine::beep(80,1000);
    }

    bgPosition = (bgPosition + 1) % 16;

    render();
    Engine::update(frameDelay);
  }

private:

  static const uint8_t W = 16;
  static const uint8_t H = 16;

  // PLAYER
  inline static int8_t playerX;
  inline static const int8_t playerY = H-1;

  // PLAYER BULLETS
  static const uint8_t maxBullets = 4;

  inline static int8_t bulletsX[maxBullets];
  inline static int8_t bulletsY[maxBullets];
  inline static uint8_t bulletsCount = 0;

  // ENEMY BULLETS
  static const uint8_t maxEnemyBullets = 3;

  inline static int8_t enemyBulletsX[maxEnemyBullets];
  inline static int8_t enemyBulletsY[maxEnemyBullets];
  inline static uint8_t enemyBulletsCount = 0;

  // ENEMIES
  static const uint8_t enemyRows = 3;

  inline static uint8_t enemyMask[enemyRows];

  inline static int8_t enemyOffsetX = 3;
  inline static int8_t enemyOffsetY = 1;
  inline static int8_t enemyDir = 1;

  inline static uint8_t enemyMoveCounter = 0;

  // UFO
  inline static int8_t ufoX = -1;

  // GAME STATE
  inline static uint16_t score = 0;
  inline static uint8_t level = 1;
  inline static bool gameOver = false;
  inline static int bgPosition = 0;

  // TIMING
  static const uint8_t frameDelay = 70;

  inline static uint8_t shootCooldown = 0;

  // INIT
  static void initPlayer() {
    playerX = W/2;
  }

  static void initEnemies() {

    for (uint8_t y=0;y<enemyRows;y++)
      enemyMask[y] = 0b00111110;

    enemyOffsetX = 3;
    enemyOffsetY = 1;
    enemyDir = 1;
  }

  // PLAYER
  static void updatePlayer() {

    playerX = constrain(playerX + Engine::getAxisX(),1,W-2);

    if(shootCooldown>0)
      shootCooldown--;

    if (Engine::getKeyDownA() && shootCooldown==0 && bulletsCount < maxBullets) {

      bulletsX[bulletsCount] = playerX;
      bulletsY[bulletsCount] = playerY-1;
      bulletsCount++;

      shootCooldown = 4;

      Engine::beep(10,1200);
    }
  }

  // BULLETS
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

  // ENEMY BULLETS
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

  // ENEMY MOVEMENT
  static void updateEnemies() {

    enemyMoveCounter++;

    uint8_t speed = max(2, 8-level);

    if (enemyMoveCounter < speed) return;

    enemyMoveCounter = 0;

    enemyOffsetX += enemyDir;

    if (enemyOffsetX <= 0 || enemyOffsetX + 6 >= W) {

      enemyDir = -enemyDir;
      enemyOffsetY++;

      enemyOffsetX += enemyDir;

      enemyShoot();
    }

    if(random(0,10)<2)
      enemyShoot();

    if (enemyOffsetY >= H-3)
      gameOver = true;
  }

  // ENEMY SHOOT
  static void enemyShoot() {

    if (enemyBulletsCount >= maxEnemyBullets) return;

    uint8_t col = random(0,6);

    for (int y=enemyRows-1;y>=0;y--) {

      if (enemyMask[y] & (1<<col)) {

        enemyBulletsX[enemyBulletsCount] = enemyOffsetX + col;
        enemyBulletsY[enemyBulletsCount] = enemyOffsetY + y;

        enemyBulletsCount++;

        Engine::beep(5,600);
        return;
      }
    }
  }

  // UFO
  static void updateUFO() {

    if(ufoX==-1 && random(0,200)==0)
      ufoX=0;

    if(ufoX!=-1)
    {
      ufoX++;

      if(ufoX>=W)
        ufoX=-1;
    }
  }

  // CHECK ENEMIES
  static bool enemiesAlive()
  {
    for(uint8_t i=0;i<enemyRows;i++)
      if(enemyMask[i])
        return true;

    return false;
  }

  // COLLISIONS
  static void checkCollisions() {

    for (uint8_t b=0;b<bulletsCount;b++) {

      int bx = bulletsX[b] - enemyOffsetX;
      int by = bulletsY[b] - enemyOffsetY;

      if (bx>=0 && bx<6 && by>=0 && by<enemyRows) {

        if (enemyMask[by] & (1<<bx)) {

          enemyMask[by] &= ~(1<<bx);

          bulletsX[b] = bulletsX[--bulletsCount];
          bulletsY[b] = bulletsY[bulletsCount];
          b--;

          score+=10;

          Engine::beep(30,1400);
        }
      }

      if(ufoX!=-1 && bulletsY[b]==0 && abs(bulletsX[b]-ufoX)<=1)
      {
        score+=50;
        ufoX=-1;
        Engine::beep(80,1800);
      }
    }

    for (uint8_t i=0;i<enemyBulletsCount;i++) {

      if (enemyBulletsY[i]==playerY &&
          abs(enemyBulletsX[i]-playerX)<=1) {

        Engine::beep(200,300);
        gameOver = true;
        return;
      }
    }
  }

  // RENDER
  static void render() {

    Engine::drawStars(0, bgPosition - 16, 16, 32, 30, Engine::color(0,0,4), Engine::color(0,0,8));

    uint32_t playerColor = Engine::color(0, 255, 0);

    Engine::setPixel(playerX,playerY - 1,playerColor);
    Engine::drawBox(playerX - 1, playerY, 3, 1, playerColor);

    for(uint8_t i=0;i<bulletsCount;i++)
      Engine::setPixel(bulletsX[i],bulletsY[i],Engine::color(255,255,0));

    for(uint8_t i=0;i<enemyBulletsCount;i++)
      Engine::setPixel(enemyBulletsX[i],enemyBulletsY[i],Engine::color(255,0,255));

    for(uint8_t y=0;y<enemyRows;y++)
    {
      for(uint8_t x=0;x<6;x++)
      {
        if(enemyMask[y]&(1<<x))
        {
          int px = enemyOffsetX + x;
          int py = enemyOffsetY + y + ((millis()/300)%2);

          Engine::setPixel(px,py,Engine::color(255,0,0));
        }
      }
    }

    if(ufoX!=-1)
    {
      Engine::setPixel(ufoX,0,Engine::color(255,255,255));
      Engine::setPixel(ufoX+1,0,Engine::color(255,0,255));
    }
  }

  static void renderGameOver()
  {
    for(int x=0;x<W;x++)
      Engine::setPixel(x,H/2,Engine::color(255,0,0));
  }
};