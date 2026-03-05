#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Engine.h"

class Rally {
public:
  // --- Initialization ---
  static void begin() {
    initRoad();
    nextStage();
  }

  // --- Main update loop ---
  static void update() {
    if (crashed) {
      crashAnimation();
      if (millis() - crashTime > 500)
        resetGame();
      return;
    }

    updateStage();
    updateCar();
    updateRoad();
    updateObstacles();
    checkCollisions();

    renderRoad();
    renderObstacles();
    renderCar();
    renderHUD();

    Engine::toneMapSpeed(speedDelay);
    Engine::update(speedDelay);
  }

private:
  // --- Constants ---
  static const int W = 16;
  static const int H = 16;
  static const int carY = 13;
  static const int roadWidth = 8;
  static const int baseDelay = 70;

  // --- Game state ---
  inline static uint8_t road[H];
  inline static int roadTarget;

  inline static float carX;
  inline static float steerVel;
  inline static bool jumping;

  inline static int speedDelay;
  inline static int distance;
  inline static int stage;

  inline static bool crashed;
  inline static unsigned long crashTime;

  inline static bool rain;
  inline static bool night;

  inline static int rockX, rockY;
  inline static int jumpX, jumpY;

  // --- Road ---
  static void initRoad() {
    for (int i = 0; i < H; i++) road[i] = 8;
  }

  static void updateRoad() {
    for (int y = H - 1; y > 0; y--) road[y] = road[y - 1];
    if (random(0, 10) == 0) {
      roadTarget += random(-2, 3);
      roadTarget = constrain(roadTarget, 4, 11);
    }
    road[0] += (roadTarget - road[0]) * 0.4;
  }

  // --- Car ---
  static void updateCar() {
    float input = Engine::getAxisX();

    steerVel = rain ? steerVel * 0.85 + input * 0.15 : input;
    carX = constrain(carX + steerVel, 1, 14);

    bool turbo = Engine::getKeyA();
    speedDelay = max(25, baseDelay - (turbo ? 25 : 0));
  }

  // --- Obstacles ---
  static void updateObstacles() {
    if (rockY < 0 && random(0, 18) == 0) { rockY = 0; rockX = road[0] + random(-2, 3); }
    if (jumpY < 0 && random(0, 40) == 0) { jumpY = 0; jumpX = road[0]; }
    if (rockY >= 0) rockY++;
    if (jumpY >= 0) jumpY++;
    if (rockY >= H) rockY = -1;
    if (jumpY >= H) { jumpY = -1; jumping = false; }
  }

  // --- Collisions ---
  static void checkCollisions() {
    int roadAtCar = road[carY];
    if (abs(carX - roadAtCar) > roadWidth / 2 && !jumping) crash();
    if (rockY == carY && abs(rockX - carX) < 1 && !jumping) crash();
    if (jumpY == carY && abs(jumpX - carX) < 1) {
      jumping = true;
      Engine::beep(40);
    }
  }

  static void crash() { crashed = true; crashTime = millis(); Engine::beep(120); }

  // --- Rendering ---
  static void renderRoad() {
    for (int y = 0; y < H; y++) {
      int c = road[y];
      for (int x = 0; x < W; x++) {
        bool onRoad = abs(x - c) <= roadWidth / 2;
        Engine::setPixel(x, y, onRoad ? Engine::color(30,30,30) : Engine::color(0,80,0));
      }
    }
  }

  static void renderObstacles() {
    if (rockY >= 0) Engine::setPixel(rockX, rockY, Engine::color(120,120,120));
    if (jumpY >= 0) Engine::setPixel(jumpX, jumpY, Engine::color(255,180,0));
  }

  static void renderCar() {
    Engine::setPixel(carX, carY, jumping ? Engine::color(255,255,255) : Engine::color(255,0,0));
    Engine::setPixel(carX, carY + 1, jumping ? Engine::color(255,255,255) : Engine::color(255,0,0));
  }

  static void renderHUD() {
    for (int i = 0; i < min(16, stage); i++) Engine::setPixel(i, 0, Engine::color(0,255,0));
  }

  static void crashAnimation() { if ((millis() / 100) % 2 == 0) renderCar(); }

  static void updateStage() { if (++distance % 200 == 0) stage++; }

  static void nextStage() { rain = false; night = false; }

  static void resetGame() {
    crashed = false;
    distance = 0;
    stage = 1;
    carX = 8;
    rockY = jumpY = -1;
    jumping = false;
    initRoad();
  }
};