#pragma once
#include <Arduino.h>
#include "Engine.h"

class Gradius {
public:
  static void begin() {
    shipX = 7; shipY = 15;
    bulletsCount = 0;
    for (int i=0;i<maxBullets;i++) bulletY[i]=-1;
    initEnemies();
    frameCounter=0;
    score=0;
    for (int y=0; y<H; y++)
      starX[y] = random(0, W);
  }

  static void update() {
    updateInput();

    frameCounter++;
    if (frameCounter>=frameDelay) {
      frameCounter=0;
      moveBullets();
      moveEnemies();
      moveStars();
      checkCollisions();
    }
    render();
    Engine::update(50);
  }

private:
  static const int W=16;
  static const int H=16;
  inline static const int frameDelay=1;
  inline static const int maxBullets=3;
  inline static const int maxEnemies=5;

  // Player
  inline static int shipX, shipY;

  // Bullets
  inline static int bulletX[maxBullets];
  inline static int bulletY[maxBullets];
  inline static int bulletsCount;

  // Enemies
  inline static int enemyX[maxEnemies];
  inline static int enemyY[maxEnemies];

  // Stars background
  inline static uint8_t starX[H];

  // Score and frame
  inline static int score;
  inline static int frameCounter;

  // --- Input ---
  static void updateInput() {
    shipX = constrain(shipX + Engine::getAxisX(), 0, W-1);
    if (Engine::getKeyDownA()) shootBullet();
  }

  // --- Bullets ---
  static void shootBullet() {
    for (int i=0;i<maxBullets;i++) {
      if (bulletY[i]<0) { bulletX[i]=shipX; bulletY[i]=shipY-1; break; }
    }
  }

  static void moveBullets() {
    for (int i=0;i<maxBullets;i++) {
      if (bulletY[i]>=0) {
        bulletY[i]--;
        if (bulletY[i]<0) bulletY[i]=-1;
      }
    }
  }

  // --- Enemies ---
  static void initEnemies() {
    for (int i=0;i<maxEnemies;i++) {
      enemyX[i] = random(0,W);
      enemyY[i] = random(-10,0);
    }
  }

  static void moveEnemies() {
    for (int i=0;i<maxEnemies;i++) {
      enemyY[i]++;
      if (enemyY[i]>=H) {
        enemyX[i] = random(0,W);
        enemyY[i] = random(-5,0);
      }
    }
  }

  // --- Stars background ---
  static void moveStars() {

    // Move everything down normally
    for (int y = H - 1; y > 0; y--) {
      starX[y] = starX[y - 1];
    }

    // New star at top
    starX[0] = random(0, W);

    // Extra shift for even rows (fake faster layer)
    for (int y = 0; y < H; y += 2) {
      if (y > 0)
        starX[y] = starX[y - 1];
    }
  }

  // --- Collisions ---
  static void checkCollisions() {
    // bullets vs enemies
    for (int b=0;b<maxBullets;b++) {
      if (bulletY[b]<0) continue;
      for (int e=0;e<maxEnemies;e++) {
        if (bulletX[b]==enemyX[e] && bulletY[b]==enemyY[e]) {
          score++;
          Engine::beep(30,1200);
          bulletY[b]=-1;
          enemyX[e]=random(0,W);
          enemyY[e]=random(-5,0);
        }
      }
    }

    // enemies vs ship
    for (int e=0;e<maxEnemies;e++)
      if (enemyX[e]==shipX && enemyY[e]==shipY) {
        begin(); // reset game
        Engine::beep(100,800);
      }
  }

  // --- Rendering ---
  static void render() {
    

    // stars
    for (int y = 0; y < H; y++) {
      int offset = (y % 2 == 0) ? frameCounter % W : 0;
      int x = (starX[y] + offset) % W;
      Engine::setPixel(x, y, Engine::color(0,0,2));
    }

    // bullets
    for (int i=0;i<maxBullets;i++)
      if (bulletY[i]>=0) Engine::setPixel(bulletX[i],bulletY[i],Engine::color(255,255,0));

    // enemies
    for (int i=0;i<maxEnemies;i++)
      Engine::setPixel(enemyX[i],enemyY[i],Engine::color(255,0,0));

    // player ship
    Engine::setPixel(shipX,shipY,Engine::color(0,0,255));

    Engine::setPixel(shipX-1,shipY,Engine::color(0,0,255));
    Engine::setPixel(shipX+1,shipY,Engine::color(0,0,255));
    Engine::setPixel(shipX,shipY-1,Engine::color(0,0,255));
  }
};