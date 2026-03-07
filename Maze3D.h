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
inline static float playerDir;
inline static int timer;

inline static bool goalReached;

static const uint8_t map[MAP_H][MAP_W] PROGMEM;

static void begin()
{
  playerX = 5.5;
  playerY = 3.5;
  playerDir = 0;
  goalReached = false;
  timer = 999;
}

    static void update()
    {
        static bool winSoundPlayed = false;
        timer--;

        if (timer % 10 == 0) {
            timerStep();
        }

        if (timer <= 0) {
            begin();
        }


        if(goalReached)
        {
            // renderWin();

            
            if(!winSoundPlayed)
            {
                playWin();
                begin();
                winSoundPlayed = true;
            }

            Engine::update(30);
            return;
        }

        handleInput();
        render();

        Engine::update(30);
    }

private:

    static void handleInput()
    {
        // rotation
        if (Engine::getAxisX() == -1)
            playerDir -= 0.1;

        if (Engine::getAxisX() == 1)
            playerDir += 0.1;

        float moveSpeed = 0.12;

        float dx = cos(playerDir);
        float dy = sin(playerDir);

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

        if (Engine::getAxisY() == -1)
        {
            nx += dx * moveSpeed;
            ny += dy * moveSpeed;
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
        tone(BUZZ_PIN, 40, 200);
    }

    static void playWin()
    {
        Engine::beep(120, 523);
        delay(120);
        Engine::beep(120, 659);
        delay(120);
        Engine::beep(200, 784);
    }

    static uint8_t isWall(float x, float y)
    {
        int mx = (int)x;
        int my = (int)y;

        if (mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H)
            return 99;

        return pgm_read_byte(&map[my][mx]);
    }

    static uint32_t getWallColor(uint8_t cell)
    {
        switch (cell)
        {
            case 1: return Engine::color(255,128,0);
            case 2: return Engine::color(128,255,0);
            case 3: return Engine::color(0,64,255);
            case 9: return Engine::color(255,255,128);
            default: return Engine::color(0,0,255);
        }
    }

    static void render()
    {
        const float FOV = 0.9;

        for(int x=0;x<16;x++)
        {
            float rayAngle = playerDir - FOV/2 + (FOV * x / 16.0);

            float rayX = playerX;
            float rayY = playerY;

            float step = 0.05;

            uint8_t hitCell = 0;
            float dist = 0;

            while(dist < 20)
            {
                rayX += cos(rayAngle) * step;
                rayY += sin(rayAngle) * step;
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

            int yStart = 8 - wallHeight/2;
            int yEnd   = 8 + wallHeight/2;

            uint32_t base = getWallColor(hitCell);

            uint8_t rr = (base >> 16) & 255;
            uint8_t gg = (base >> 8) & 255;
            uint8_t bb = base & 255;

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

            uint32_t color = Engine::color(rr,gg,bb);

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

        Engine::drawNumber3x4(0, 12, timer / 10, Engine::gray);
    }   

};


const uint8_t Maze3D::map[21][32] PROGMEM =
{
  // Row 0
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  // Row 1
  {1,0,0,0,2,0,0,1,0,3,0,0,1,0,0,0,2,0,0,1,0,0,3,0,0,1,0,0,2,0,0,1},
  // Row 2
  {1,0,1,1,0,1,0,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,1,0,1},
  // Row 3
  {1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,1},
  // Row 4
  {1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
  // Row 5
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 6
  {1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,1,1},
  // Row 7
  {1,0,0,0,0,0,0,2,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 8
  {1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
  // Row 9
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 10
  {1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,1},
  // Row 11
  {1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 12
  {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
  // Row 13
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 14
  {1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,1},
  // Row 15
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,3,0,0,0,0,0,1},
  // Row 16
  {1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1},
  // Row 17
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 18
  {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1},
  // Row 19
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9},
  // Row 20
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};