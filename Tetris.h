#pragma once
#include <Arduino.h>
#include "Engine.h"

class Tetris {
public:
  static void begin() {
    randomSeed(analogRead(A5));
    initGrid();
    nextTetType = random(0, 7);
    spawnTetromino();
    frameCounter = 0;
    score = 0;
    // Music::play(0, SONG_CH0, true); // true = looping
    // Music::play(1, SONG_CH1, true); // true = looping
    // Music::play(2, SONG_CH2, true); // true = looping
  }

  static void update() {
    updateInput();

    frameCounter++;
    if (frameCounter >= dropDelay) {
      frameCounter = 0;
      moveTetromino(0, 1);
    }

    render();
    Engine::update(frameDelay);
  }

private:
  static const uint8_t W = 10;  // playfield width (grid)
  static const uint8_t H = 16;  // playfield height

  static const uint8_t frameDelay = 60;
  static const uint8_t dropDelay = 6;

  inline static uint8_t grid[H][W];

  inline static const uint8_t tetrominoes[7][4][4] PROGMEM = {
    { { 0, 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },  // I
    { { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },  // O
    { { 0, 1, 0, 0 }, { 1, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },  // T
    { { 0, 0, 1, 0 }, { 1, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },  // L
    { { 1, 0, 0, 0 }, { 1, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },  // J
    { { 0, 1, 1, 0 }, { 1, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },  // S
    { { 1, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }   // Z
  };

// --- LEAD (Melody) ---
inline static const char SONG_CH0[] PROGMEM =
"C2E2G2C+2B2G2E2C2 "
"D2F2A2D+2C+2A2F2D2 "
"E2G2B2E+2D+2B2G2E2 "
"C+2B2A2G2E2C2R2R2 "

"G2E2C2E2G2C+2B2G2 "
"A2F2D2F2A2D+2C+2A2 "
"B2G2E2G2B2E+2D+2B2 "
"C+4R4";

// --- BASS (low + punchy) ---
inline static const char SONG_CH1[] PROGMEM =
"C4C4G3G3A3A3F3F3 "
"D4D4A3A3B3B3G3G3 "
"E4E4B3B3C4C4A3A3 "
"C4C4G3G3C4C4R4R4 "

"C4R4G3R4A3R4F3R4 "
"D4R4A3R4B3R4G3R4 "
"E4R4B3R4C4R4A3R4 "
"C4R4R4R4";

// --- HARMONY / ARPEGGIO (fast sparkle) ---
inline static const char SONG_CH2[] PROGMEM =
"C1E1G1C+1E1G1C+1E1 "
"D1F1A1D+1F1A1D+1F1 "
"E1G1B1E+1G1B1E+1G1 "
"C+1G1E1C1G1E1C1R1 "

"G1B1D+1G+1D+1B1G1D+1 "
"A1C+1E+1A+1E+1C+1A1E+1 "
"B1D+1F+1B+1F+1D+1B1F+1 "
"C+2R2";

  inline static int8_t tetX, tetY;
  inline static uint8_t tetType, tetRotation;
  inline static uint8_t nextTetType;

  inline static uint8_t frameCounter;
  inline static uint16_t score;

  // ---------------- GRID ----------------
  static void initGrid() {
    for (uint8_t y = 0; y < H; y++)
      for (uint8_t x = 0; x < W; x++)
        grid[y][x] = 0;
  }

  // ---------------- SPAWN ----------------
  static void spawnTetromino() {
    tetType = nextTetType;
    nextTetType = random(0, 7);
    tetRotation = 0;
    tetX = W / 2 - 2;
    tetY = 0;
  }

  // ---------------- INPUT ----------------
  static void updateInput() {
    int8_t dx = Engine::getAxisX();
    int8_t dy = Engine::getAxisY();

    if (dx != 0) moveTetromino(dx, 0);
    if (Engine::getKeyDownB()) {  // rotate counter
      rotateTetromino();
      rotateTetromino();
      rotateTetromino();
    }
    if (Engine::getKeyA() || dy == 1) moveTetromino(0, 1);
  }

  // ---------------- MOVEMENT ----------------
  static void moveTetromino(int8_t dx, int8_t dy) {
    tetX += dx;
    tetY += dy;

    if (checkCollision()) {
      tetX -= dx;
      tetY -= dy;

      if (dy > 0) {
        lockTetromino();
        clearLines();
        spawnTetromino();
        if (checkCollision()) {  // game over
          initGrid();
          score = 0;
        }
      }
    }
  }

  static void rotateTetromino() {
    tetRotation = (tetRotation + 1) % 4;
    if (checkCollision()) tetRotation = (tetRotation + 3) % 4;
  }

  // ---------------- COLLISION ----------------
  static bool checkCollision() {
    for (uint8_t y = 0; y < 4; y++)
      for (uint8_t x = 0; x < 4; x++) {
        if (!getBlock(x, y)) continue;
        int8_t px = tetX + x;
        int8_t py = tetY + y;
        if (px < 0 || px >= W || py < 0 || py >= H) return true;
        if (grid[py][px]) return true;
      }
    return false;
  }

  static bool getBlock(uint8_t x, uint8_t y) {
    uint8_t rx = x, ry = y;
    for (uint8_t r = 0; r < tetRotation; r++) {
      uint8_t tmp = rx;
      rx = 3 - ry;
      ry = tmp;
    }
    return pgm_read_byte(&tetrominoes[tetType][ry][rx]);
  }

  // ---------------- LOCK ----------------
  static void lockTetromino() {
    for (uint8_t y = 0; y < 4; y++)
      for (uint8_t x = 0; x < 4; x++)
        if (getBlock(x, y)) {
          int8_t px = tetX + x;
          int8_t py = tetY + y;
          if (px >= 0 && px < W && py >= 0 && py < H)
            grid[py][px] = 1;
        }
    Engine::beep(100, 1000);
  }

  // ---------------- LINE CLEAR ----------------
  static void clearLines() {
    uint8_t clearedLines = 0;
    for (int8_t y = H - 1; y >= 0; y--) {
      bool full = true;
      for (uint8_t x = 0; x < W; x++)
        if (!grid[y][x]) full = false;

      if (full) {
        for (int8_t yy = y; yy > 0; yy--)
          for (uint8_t x = 0; x < W; x++)
            grid[yy][x] = grid[yy - 1][x];
        for (uint8_t x = 0; x < W; x++)
          grid[0][x] = 0;

        clearedLines++;
        y++;
      }
    }
    switch (clearedLines) {
      case 3:
        score += 5;
        break;
      case 4:
        score += 10;
        break;
      default:
        score += clearedLines;
        break;
    }
    if (clearedLines > 0) {
      Engine::beep(150, 1500);
    }
  }

  // ---------------- RENDER ----------------
  static void render() {
    // --- vertical gray borders ---
    for (uint8_t y = 0; y < H; y++) {
      Engine::setPixel(0, y, Engine::color(100, 100, 100));      // left border
      Engine::setPixel(W + 1, y, Engine::color(100, 100, 100));  // right border
    }

    // draw locked blocks
    for (uint8_t y = 0; y < H; y++)
      for (uint8_t x = 0; x < W; x++)
        if (grid[y][x])
          Engine::setPixel(x + 1, y, Engine::color(255, 128, 0));  // +1 for left border

    // draw current piece
    for (uint8_t y = 0; y < 4; y++)
      for (uint8_t x = 0; x < 4; x++)
        if (getBlock(x, y)) {
          int8_t px = tetX + x;
          int8_t py = tetY + y;
          if (px >= 0 && px < W && py >= 0 && py < H)
            Engine::setPixel(px + 1, py, Engine::color(0, 255, 0));  // +1 for border
        }

    // draw next piece preview (outside main grid)
    uint8_t previewY = 2;
    for (uint8_t y = 0; y < 4; y++)
      for (uint8_t x = 0; x < 4; x++)
        if (pgm_read_byte(&tetrominoes[nextTetType][y][x]))
          Engine::setPixel(W + 2 + x, previewY + y, Engine::color(0, 0, 255));

    Engine::drawNumber3x4Vertical(W + 2, previewY + 5, score, Engine::color(0, 0, 255));

    // draw score as vertical bars (outside main grid)
    uint16_t tempScore = score;
    uint8_t baseX = W + 6;
    for (uint8_t i = 0; i < 4; i++) {
      uint8_t digit = tempScore % 10;
      tempScore /= 10;

      for (uint8_t d = 0; d < digit && d < H; d++)
        Engine::setPixel(baseX + i, H - 1 - d, Engine::color(255, 255, 0));
    }
  }
};