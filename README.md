# 🎮 ArduConsole

**ArduConsole** is a tiny retro gaming console built with an **Arduino**, a **16x16 WS2812B addressable LED matrix**, and a **joystick shield**.

Despite the tiny display, the console runs multiple classic arcade-style games with sound effects and responsive controls.

It is designed to be simple, portable, and fun to build.

---

# 📺 Demo

https://www.youtube.com/watch?v=S-Pd8go_0Wc

---

# 🧠 Features

- 16×16 RGB LED graphics
- Multiple built-in arcade games
- Joystick + button controls
- Sound effects via piezo buzzer
- Pause system
- Game switching system
- Simple sprite rendering engine
- Written in clean modular C++ classes

---

# 🕹️ Games Included

The console currently includes the following games:

| Game | Description |
|-----|-------------|
| **Arkanoid** | Break the bricks with the bouncing ball |
| **Space Invaders** | Defend Earth from alien invaders |
| **Rally** | Retro top-down racing |
| **Tetris** | Classic falling block puzzle |
| **Pac-Man** | Maze chasing gameplay |
| **Gradius** | Side-scrolling space shooter |
| **Pong** | Classic paddle tennis |
| **River Raid** | Fly through a dangerous river canyon |
| **Frogger** | Help the frog cross the road and river |

---

# 🧩 Hardware Required

- Arduino (Uno / compatible)
- **WS2812B 16×16 LED Matrix**
- **Joystick Shield**
- **Piezo Buzzer**
- Wires / Breadboard (optional)
- 5V power supply

---

# 🔌 Pin Configuration

```cpp
#define LED_PIN 12
#define BUZZ_PIN 11
#define JOY_PIN_X A0
#define JOY_PIN_Y A1
#define BTN_PIN_A 4
#define BTN_PIN_B 3
#define BTN_PIN_X 5
#define BTN_PIN_Y 2
#define BTN_PIN_Z 8
```

### Connections

| Component | Arduino Pin |
|----------|-------------|
| WS2812B Matrix Data | 12 |
| Buzzer | 11 |
| Joystick X | A0 |
| Joystick Y | A1 |
| Button A | 4 |
| Button B | 3 |
| Button X | 5 |
| Button Y | 2 |
| Button Z | 8 |

---

# 🎮 Controls

| Control | Action |
|-------|--------|
| Joystick | Move player |
| **Button X** | Switch game |
| **Button Y** | Pause / Resume |
| Other buttons | Game specific actions |

---

# 🛠️ Libraries Used

- **Adafruit NeoPixel**

Install via Arduino Library Manager.

---

# 📁 Project Structure

```
ArduConsole
│
├── Engine.h
├── SplashScreens.h
│
├── Arkanoid.h
├── SpaceInvaders.h
├── Rally.h
├── Tetris.h
├── PacMan.h
├── Gradius.h
├── Pong.h
├── RiverRaid.h
├── Frogger.h
│
└── ArduConsole.ino
```

Each game is implemented as a separate class with:

```
static void begin();
static void update();
```

This makes it easy to add new games.

---

# ▶️ How It Works

1. Console boots and shows a **splash screen animation**.
2. All games are initialized.
3. The main loop runs the **current selected game**.
4. Press **X** to switch to the next game.
5. Press **Y** to pause or resume gameplay.

---

# 🚀 Adding New Games

To add a new game:

1. Create a new class:

```
class MyGame {
public:
  static void begin();
  static void update();
};
```

2. Include it in the main file:

```
#include "MyGame.h"
```

3. Initialize it in `setup()`:

```
MyGame::begin();
```

4. Add it to the game switch block in `loop()`.

---

# 🧑‍💻 Author

Created by **Tigran Niksalyan**

---

# ⭐ Future Ideas

- Score system
- High score saving
- More games
- Multiplayer Pong
- Menu system
- Soundtracks
- Animations

---

# 📜 License

Open source. Use it, modify it, improve it, and build your own tiny arcade machine.
