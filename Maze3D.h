#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "Engine.h"

class Maze3D {
public:

static const int MAP_W = 32;
static const int MAP_H = 21;

static float playerX;
static float playerY;
static float playerDir;

static const uint8_t map[MAP_H][MAP_W] PROGMEM;

static void begin()
{
  playerX = 5.5;
  playerY = 3.5;
  playerDir = 0;
}

static void update()
{
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

    if (!isWall(nx, ny))
    {
        playerX = nx;
        playerY = ny;
    }
}

static bool isWall(float x, float y)
{
    int mx = (int)x;
    int my = (int)y;

    if (mx < 0 || my < 0 || mx >= MAP_W || my >= MAP_H)
        return true;

    return pgm_read_byte(&map[my][mx]) != 0;
}

static uint32_t getWallColor(uint8_t cell)
{
    switch (cell)
    {
        case 1: return Engine::color(255,0,0);
        case 2: return Engine::color(255,128,0);
        case 3: return Engine::color(0,64,255);
        default: return Engine::color(255,255,255);
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

        uint8_t shade = 255 - min(dist * 120, 240);
        uint32_t base = getWallColor(hitCell);

        uint8_t r = ((base >> 16) & 255) * shade / 255;
        uint8_t g = ((base >> 8) & 255) * shade / 255;
        uint8_t b = (base & 255) * shade / 255;

        uint32_t color = Engine::color(r,g,b);

        for(int y=0;y<16;y++)
        {
            if(y < yStart)
                Engine::setPixel(x,y, Engine::color(10,10,30));
            else if(y > yEnd)
                Engine::setPixel(x,y, Engine::color(0,20,0));
            else
                Engine::setPixel(x,y, color);
        }
    }
}

};

float Maze3D::playerX;
float Maze3D::playerY;
float Maze3D::playerDir;

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
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  // Row 20
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};