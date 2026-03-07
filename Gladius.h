#pragma once
#include <Arduino.h>
#include "Engine.h"

class Gladius {
public:

  static void begin() {
    shipX = W/3;
    shipY = H/2;

    shootCooldown = 0;
    score = 0;
    gameOver = false;

    for(int i=0;i<maxBullets;i++) bulletY[i] = -1;

    initEnemies();
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

    updateInput();

    frameCounter++;

    if(frameCounter>=frameDelay)
    {
      frameCounter=0;

      moveBullets();
      moveEnemies();
      checkCollisions();
    }

    render();

    Engine::update(50);
  }

private:

  static const int W=16;
  static const int H=16;

  static const int frameDelay=1;
  static const int maxBullets=3;
  static const int maxEnemies=5;

  // Player
  inline static int shipX, shipY;

  // Bullets
  inline static int bulletX[maxBullets];
  inline static int bulletY[maxBullets];

  // Enemies
  inline static int enemyX[maxEnemies];
  inline static int enemyY[maxEnemies];
  inline static int enemyType[maxEnemies];

  // Game state
  inline static int score;
  inline static int frameCounter;
  inline static int shootCooldown;
  inline static bool gameOver;

  // -------- INPUT --------

  static void updateInput()
  {
    shipX = constrain(shipX + Engine::getAxisX(),1,W-2);
    shipY = constrain(shipY + Engine::getAxisY(),1,H-2);

    if(shootCooldown>0)
      shootCooldown--;

    if(Engine::getKeyDownA() && shootCooldown==0)
    {
      shootBullet();
      shootCooldown = 3;
      Engine::beep(10,1400);
    }
  }

  // -------- BULLETS --------

  static void shootBullet()
  {
    for(int i=0;i<maxBullets;i++)
    {
      if(bulletY[i]<0)
      {
        bulletX[i]=shipX+1;
        bulletY[i]=shipY;
        return;
      }
    }
  }

  static void moveBullets()
  {
    for(int i=0;i<maxBullets;i++)
    {
      if(bulletY[i]>=0)
      {
        bulletX[i]++;

        if(bulletX[i] >= W)
          bulletY[i] = -1;
      }
    }
  }

  // -------- ENEMIES --------

  static void initEnemies()
  {
    for(int i=0;i<maxEnemies;i++)
    {
      enemyX[i]=W + random(10);
      enemyY[i]=random(1,H-2);
      enemyType[i]=random(0,2);
    }
  }

  static void moveEnemies()
  {
    int speed = 1 + score/20;

    for(int i=0;i<maxEnemies;i++)
    {
      enemyX[i] -= speed;

      if(enemyType[i]==1)
        enemyY[i] += (millis()/200)%3-1;

      enemyY[i] = constrain(enemyY[i],0,H-1);

      if(enemyX[i] < 0)
      {
        enemyX[i] = W + random(3);
        enemyY[i] = random(1,H-2);
        enemyType[i] = random(0,2);
      }
    }
  }

  // -------- COLLISIONS --------

  static void checkCollisions()
  {
    for(int b=0;b<maxBullets;b++)
    {
      if(bulletY[b]<0) continue;

      for(int e=0;e<maxEnemies;e++)
      {
        if(bulletX[b]==enemyX[e] &&
           bulletY[b]==enemyY[e])
        {
          score++;

          bulletY[b]=-1;

          enemyX[e]=random(0,W);
          enemyY[e]=random(-5,0);

          Engine::beep(20,1600);
        }
      }
    }

    for(int e=0;e<maxEnemies;e++)
    {
      if(enemyX[e]==shipX &&
         enemyY[e]==shipY)
      {
        Engine::beep(120,400);
        gameOver = true;
      }
    }
  }

  // -------- STARFIELD --------

  static void drawStars(int x,int y,int w,int h,
                        uint8_t density,
                        uint32_t dimColor,
                        uint32_t brightColor)
  {
    for(int iy=0;iy<h;iy++)
    {
      for(int ix=0;ix<w;ix++)
      {
        uint8_t lx = (ix + (millis()>>4)) & 15;
        uint8_t ly = (iy + (millis()>>5)) & 15;

        uint8_t r = lx * 197 + ly * 101;
        r ^= r >> 3;
        r ^= lx * 29;
        r ^= ly * 71;
        r *= 53;
        r ^= r >> 4;

        if(r < density)
        {
          if(r < (density>>2))
            Engine::setPixel(x+ix,y+iy,brightColor);
          else
            Engine::setPixel(x+ix,y+iy,dimColor);
        }
      }
    }
  }

  // -------- RENDER --------

  static void render()
  {
    drawStars(0,0,W,H,
      10,
      Engine::color(0,0,4),
      Engine::color(30,30,30));

    for(int i=0;i<maxBullets;i++)
      if(bulletY[i]>=0)
        Engine::setPixel(bulletX[i],bulletY[i],
          Engine::color(255,255,0));

    for(int i=0;i<maxEnemies;i++)
    {
      uint32_t c =
        enemyType[i] ?
        Engine::color(255,0,100) :
        Engine::color(255,40,0);

      Engine::setPixel(enemyX[i],enemyY[i],c);
    }

    uint32_t pc = Engine::color(0,0,255);

    Engine::setPixel(shipX,shipY,pc);
    Engine::setPixel(shipX-1,shipY,pc);
  }

  static void renderGameOver()
  {
    drawStars(0,0,W,H,6,
      Engine::color(0,0,4),
      Engine::color(30,30,30));

    for(int x=0;x<W;x++)
      Engine::setPixel(x,H/2,
        Engine::color(255,0,0));
  }
};