#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Engine.h"


#include "Rally.h"
#include "SpaceInvaders.h"
#include "Arkanoid.h"
#include "Tetris.h"
#include "PacMan.h"
#include "Gradius.h"
#include "Pong.h"
#include "RiverRaid.h"
#include "Frogger.h"
#include "Maze3D.h"

#include "SplashScreens.h"


int currentGame = 0;
bool isPaused = false;


void setup() {
  Engine::begin();

  initialize();

  Engine::cls();
}

void loop() {

  if (Engine::getKeySelect() && Engine::getKeyStart()) {
    initialize();
  }

  if (!isPaused) {
    switch (currentGame) {
      case 0:
        Arkanoid::update();
        break;
      case 1:
        SpaceInvaders::update();
        break;
      case 2:
        Rally::update();
        break;
      case 3:
        Tetris::update();
        break;
      case 4:
        PacMan::update();
        break;
      case 5:
        Gradius::update();
        break;
      case 7:
        Pong::update();
        break;
      case 8:
        RiverRaid::update();
        break;
      case 9:
        Frogger::update();
        break;
      case 10:
        Maze3D::update();
        break;
    }
  } else {
    delay(50);
    Engine::updateKeys();
  }


  if (Engine::getKeyDownStart()) {
    isPaused = !isPaused;
    // Short rapid beeps
    if (isPaused) {
      Engine::beep(60, 500);
      delay(10);
      Engine::beep(60, 1000);
      delay(10);
      Engine::beep(60, 750);  // slight drop
      delay(10);
      Engine::beep(100, 1500);  // final longer lower beep
    }
  }

  if (!isPaused && Engine::getKeyUpSelect()) {
    Engine::cls();
    currentGame = (currentGame + 1) % 11;
  }

  if (Engine::getKeyDownX()) {
    Engine::mute = !Engine::mute;
  }
}

void initialize() {

  isPaused = false;
  for (uint8_t fd = 0; fd < 50; fd++) {
    Engine::drawSprite(&SPLASH_IMAGE_2[0][0], &SPLASH_PALETTE_2[0][0], 16, 16, 0, 0, true);

    if (fd >= 15) {
      Engine::drawSprite(&SPLASH_IMAGE_4[0][0], &SPLASH_PALETTE_4[0][0], 8, 8, 4, 4, true);
    }

    if (fd >= 13 && fd <= 15) {
      Engine::drawSprite(&SPLASH_IMAGE_5[0][0], &SPLASH_PALETTE_5[0][0], 8, 8, 4, 4, true);
    }

    if (fd == 15) {

      Engine::beep(25, 880);   // A5
      Engine::beep(25, 988);   // B5
      Engine::beep(25, 1175);  // D6
      Engine::beep(40, 1568);  // G6
      delay(20);
    }

    Engine::update(50);
  }

  Rally::begin();
  SpaceInvaders::begin();
  Arkanoid::begin();
  Tetris::begin();
  PacMan::begin();
  Gradius::begin();
  Pong::begin();
  RiverRaid::begin();
  Frogger::begin();
  Maze3D::begin();
}
