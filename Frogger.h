#pragma once
#include <Arduino.h>
#include "Engine.h"

class Frogger {
public:
  static void begin() {
    frogX = W / 2;
    frogY = H - 1;

    score = 0;
    frameCounter = 0;

    for (int i = 0; i < maxCars; i++) {
      carX[i] = random(0, W);
      carDir[i] = random(0, 2) ? 1 : -1;
      carLane[i] = 10 + (i % 3); // road lanes
    }

    for (int i = 0; i < maxLogs; i++) {
      logX[i] = random(0, W);
      logDir[i] = random(0, 2) ? 1 : -1;
      logLane[i] = 4 + (i % 3); // river lanes
      logLen[i] = random(2,4);
    }
  }

  static void update() {
    updateInput();

    frameCounter++;
    if (frameCounter >= frameDelay) {
      frameCounter = 0;
      moveCars();
      moveLogs();
      checkRiver();
      checkCollisions();
    }

    render();
    Engine::update(60);
  }

private:
  static const int W = 16;
  static const int H = 16;

  inline static const int frameDelay = 2;
  inline static const int maxCars = 5;
  inline static const int maxLogs = 5;

  inline static int frogX;
  inline static int frogY;

  inline static int score;
  inline static int frameCounter;

  inline static int carX[maxCars];
  inline static int carLane[maxCars];
  inline static int carDir[maxCars];

  inline static int logX[maxLogs];
  inline static int logLane[maxLogs];
  inline static int logDir[maxLogs];
  inline static int logLen[maxLogs];

  // -------- INPUT --------
  static void updateInput() {
    int dx = Engine::getAxisX();
    int dy = Engine::getAxisY();

    if (dx != 0 || dy != 0) {
      frogX = constrain(frogX + dx, 0, W - 1);
      frogY = constrain(frogY + dy, 0, H - 1);
      Engine::beep(10, 900);
    }
  }

  // -------- CARS --------
  static void moveCars() {
    for (int i = 0; i < maxCars; i++) {
      carX[i] += carDir[i];

      if (carDir[i] == 1 && carX[i] >= W)
        carX[i] = -2;

      if (carDir[i] == -1 && carX[i] < -2)
        carX[i] = W;
    }
  }

  // -------- LOGS --------
  static void moveLogs() {
    for (int i = 0; i < maxLogs; i++) {
      logX[i] += logDir[i];

      if (logDir[i] == 1 && logX[i] >= W)
        logX[i] = -logLen[i];

      if (logDir[i] == -1 && logX[i] < -logLen[i])
        logX[i] = W;
    }
  }

  // -------- RIVER CHECK --------
  static void checkRiver() {
    if (frogY >= 4 && frogY <= 6) {

      bool onLog = false;

      for (int i = 0; i < maxLogs; i++) {
        if (frogY == logLane[i]) {
          for (int l = 0; l < logLen[i]; l++) {
            if (frogX == logX[i] + l) {
              onLog = true;
              frogX += logDir[i];
              frogX = constrain(frogX, 0, W - 1);
            }
          }
        }
      }

      if (!onLog) {
        splash();
      }
    }
  }

  // -------- COLLISIONS --------
  static void checkCollisions() {

    for (int i = 0; i < maxCars; i++) {
      if (frogY == carLane[i] &&
          frogX >= carX[i] &&
          frogX <= carX[i] + 1) {
        die();
      }
    }

    if (frogY == 0) {
      score++;
      Engine::beep(80, 1500);
      frogX = W / 2;
      frogY = H - 1;
    }
  }

  static void splash() {
    Engine::beep(120, 300);
    frogX = W / 2;
    frogY = H - 1;
  }

  static void die() {
    Engine::beep(150, 200);
    frogX = W / 2;
    frogY = H - 1;
  }

  // -------- RENDER --------
  static void render() {

    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {

        if (y >= 4 && y <= 6)
          Engine::setPixel(x, y, Engine::color(0,0,80)); // river
        else if (y >= 10 && y <= 12)
          Engine::setPixel(x, y, Engine::color(40,40,40)); // road
        else
          Engine::setPixel(x, y, Engine::color(0,80,0)); // grass
      }
    }

    // logs
    for (int i = 0; i < maxLogs; i++)
      for (int l = 0; l < logLen[i]; l++)
        Engine::setPixel(logX[i] + l, logLane[i], Engine::color(120,70,20));

    // cars
    for (int i = 0; i < maxCars; i++) {
      Engine::setPixel(carX[i], carLane[i], Engine::color(255,0,0));
      Engine::setPixel(carX[i]+1, carLane[i], Engine::color(255,120,0));
    }

    // frog
    Engine::setPixel(frogX, frogY, Engine::color(0,255,0));

    // score
    for (int i = 0; i < min(score, H); i++)
      Engine::setPixel(W-1, H-1-i, Engine::color(255,255,0));
  }
};