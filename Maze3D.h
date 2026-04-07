#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Engine.h"

class Maze3D {
public:

static const int MAP_W = 32;
static const int MAP_H = 21;

inline static float playerX;
inline static float playerY;
inline static uint8_t playerAngle;
inline static int timer;

inline static bool goalReached;

static const uint8_t map[MAP_H][MAP_W] PROGMEM;
static const int16_t sinQ15Table[256] PROGMEM;

static void begin()
{
  playerX = 5.5;
  playerY = 3.5;
  playerAngle = 135;
  goalReached = false;
  timer = 999;
}

    static void update()
    {
        timer--;

        if (timer % 10 == 0) {
            timerStep();
        }

        if (timer <= 0) {
            playLose();
            begin();
            Engine::cls();
            return;
        }


        if(goalReached)
        {
            playWin();
            begin();
            Engine::cls();
            return;
        }

        handleInput();
        render();

        Engine::update(30);
    }

private:
    static constexpr float q15ToFloat = 1.0f / 32768.0f;
    // 256 steps around the circle: 1 step ≈ 2π/256 rad ≈ 0.02454 rad
    static constexpr uint8_t turnStep = 4; // ~0.098 rad per input tick
    static constexpr uint8_t fovUnits = 37; // ~0.9 rad

    static inline float sinFast(uint8_t angle)
    {
        return (int16_t)pgm_read_word(&sinQ15Table[angle]) * q15ToFloat;
    }

    static inline float cosFast(uint8_t angle)
    {
        // cos(x) = sin(x + π/2) => +64 in 0..255 units
        return sinFast((uint8_t)(angle + 64));
    }

    static void handleInput()
    {
        // rotation
        if (Engine::getAxisX() == -1)
            playerAngle -= turnStep;

        if (Engine::getAxisX() == 1)
            playerAngle += turnStep;

        float moveSpeed = 0.12;

        float dx = cosFast(playerAngle);
        float dy = sinFast(playerAngle);

        float nx = playerX;
        float ny = playerY;

        // forward
        if (Engine::getAxisY() == -1)
        {
            nx += dx * moveSpeed;
            ny += dy * moveSpeed;
            playStep();
        }

        // backward
        if (Engine::getAxisY() == 1)
        {
            nx -= dx * moveSpeed;
            ny -= dy * moveSpeed;
        }

        uint8_t wall = isWall(nx, ny);
        if (wall == 0)
        {
            playerX = nx;
            playerY = ny;
        }
        else
        {
            playBump();
        }

        // check goal
        if(wall == 9)
        {
            goalReached = true;
        }
    }

    static void playStep()
    {
        Engine::beep(20, 900);
    }

    static void timerStep()
    {
        Engine::beep(20, 100);
    }

    static void playBump()
    {
        Engine::beep(200, 40);
    }

    static void playWin()
    {
        Engine::beep(120, 523);
        delay(120);
        Engine::beep(120, 659);
        delay(120);
        Engine::beep(200, 784);
    }

    static void playLose()
    {
        Engine::beep(180, 784);
        delay(120);
        Engine::beep(200, 523);
        delay(120);
        Engine::beep(260, 196);
    }

    static uint8_t isWall(float x, float y)
    {
        int mx = (int)x;
        int my = (int)y;

        if (mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H)
            return 255;

        return pgm_read_byte(&map[my][mx]);
    }

    static CRGB getWallColor(uint8_t cell)
    {
        switch (cell)
        {
            case 1: return Engine::color(255,190,0);
            case 2: return Engine::color(0,0,255);
            case 3: return Engine::color(255,0,0);
            case 9: return Engine::color(0,255,0);
            default: return Engine::color(255,0,0);
        }
    }

    static void render()
    {
        const float step = 0.05f;
        const float maxDist = 20.0f;

        for(int x=0;x<16;x++)
        {
            uint8_t rayAngle = (uint8_t)(playerAngle - (fovUnits / 2) + (uint8_t)((fovUnits * x) / 16));

            float rayX = playerX;
            float rayY = playerY;

            uint8_t hitCell = 0;
            float dist = 0;

            const float dirX = cosFast(rayAngle);
            const float dirY = sinFast(rayAngle);

            while(dist < maxDist)
            {
                rayX += dirX * step;
                rayY += dirY * step;
                dist += step;

                int mx = (int)rayX;
                int my = (int)rayY;

                if(mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H)
                    break;

                uint8_t cell = pgm_read_byte(&map[my][mx]);

                if(cell != 0)
                {
                    hitCell = cell;
                    break;
                }
            }

            if (dist < 0.1) dist = 0.1;

            int wallHeight = (int)(14.0 / dist);

            if(wallHeight > 16) wallHeight = 16;

            int yStart = 7 - wallHeight/2;
            int yEnd   = 7 + wallHeight/2;

            CRGB base = getWallColor(hitCell);

            uint8_t rr = base.r;;
            uint8_t gg = base.g;
            uint8_t bb = base.b;

            // distance shading
            uint8_t shade = 255 - min(dist * 120, 240);
            rr = rr * shade / 255;
            gg = gg * shade / 255;
            bb = bb * shade / 255;

            // stronger fog darkness
            if(dist > 5)
            {
                rr *= 0.7;
                gg *= 0.7;
                bb *= 0.7;
            }

            if(dist > 7)
            {
                rr *= 0.45;
                gg *= 0.45;
                bb *= 0.45;
            }

            if(dist > 9)
            {
                rr *= 0.25;
                gg *= 0.25;
                bb *= 0.25;
            }

            if(dist > 12)
            {
                rr *= 0.12;
                gg *= 0.12;
                bb *= 0.12;
            }

            CRGB color = Engine::color(rr,gg,bb);

            for(int y=0;y<16;y++)
            {
                if(y < yStart)
                {
                    // sky
                    Engine::setPixel(x,y, Engine::color(8,8,25));
                }
                else if(y > yEnd)
                {
                    // floor gradient
                    uint8_t f = 10 + (y * 2);
                    Engine::setPixel(x,y, Engine::color(0,f,0));
                }
                else
                {
                    // cheap wall texture pattern
                    if((x + y) % 3 == 0)
                    {
                        uint8_t tr = rr * 0.8;
                        uint8_t tg = gg * 0.8;
                        uint8_t tb = bb * 0.8;
                        Engine::setPixel(x,y, Engine::color(tr,tg,tb));
                    }
                    else
                    {
                        Engine::setPixel(x,y, color);
                    }
                }
            }
        }

        Engine::drawNumber3x4(0, 12, timer / 10, timer > 99 ? CRGB::Gray : CRGB::Red);
    }   

};

// Q15 sine lookup table: sin(2π*i/256) * 32767
const int16_t Maze3D::sinQ15Table[256] PROGMEM = {
       0,    804,   1608,   2410,   3212,   4011,   4808,   5602,
    6393,   7179,   7962,   8739,   9512,  10278,  11039,  11793,
   12539,  13279,  14010,  14732,  15446,  16151,  16846,  17530,
   18204,  18868,  19519,  20159,  20787,  21403,  22005,  22594,
   23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,
   27245,  27683,  28105,  28510,  28898,  29268,  29621,  29956,
   30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
   32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,
   32767,  32757,  32728,  32678,  32609,  32521,  32412,  32285,
   32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,
   30273,  29956,  29621,  29268,  28898,  28510,  28105,  27683,
   27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,
   23170,  22594,  22005,  21403,  20787,  20159,  19519,  18868,
   18204,  17530,  16846,  16151,  15446,  14732,  14010,  13279,
   12539,  11793,  11039,  10278,   9512,   8739,   7962,   7179,
    6393,   5602,   4808,   4011,   3212,   2410,   1608,    804,
       0,   -804,  -1608,  -2410,  -3212,  -4011,  -4808,  -5602,
   -6393,  -7179,  -7962,  -8739,  -9512, -10278, -11039, -11793,
  -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530,
  -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
  -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
  -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
  -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971,
  -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
  -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285,
  -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
  -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
  -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
  -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868,
  -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
  -12539, -11793, -11039, -10278,  -9512,  -8739,  -7962,  -7179,
   -6393,  -5602,  -4808,  -4011,  -3212,  -2410,  -1608,   -804,
};


const uint8_t Maze3D::map[21][32] PROGMEM =
{
  // Legend:
  // 0 = empty, 1/2/3 = wall (different colors), 9 = goal
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1},
  {1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,1},
  {1,0,1,0,2,0,1,0,3,0,1,0,1,0,2,0,1,0,3,0,1,0,1,0,2,0,1,0,3,1,0,1},
  {1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,1},
  {1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1},
  {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
  {1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1},
  {1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1},
  {1,0,1,0,2,0,1,1,1,0,1,0,1,0,2,2,2,0,1,0,3,1,1,0,1,0,2,0,3,1,0,1},
  {1,0,1,0,0,0,1,0,0,0,1,0,1,0,2,0,2,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1},
  {1,0,1,1,1,0,1,0,1,1,1,0,1,0,2,0,2,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1},
  {1,0,0,0,1,0,0,0,1,0,0,0,1,0,2,0,2,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1},
  {1,1,1,0,1,1,1,0,1,0,1,1,1,0,2,0,2,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
  {1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,1},
  {1,0,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,0,1,1,0,1,0,1},
  {1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,1,0,1},
  {1,0,1,0,3,3,3,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,3,0,1,0,3,3,0,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};