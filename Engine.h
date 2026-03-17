#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>


// Pins and constants
#define LED_PIN 12
#define BUZZ_PIN 11
#define JOY_PIN_X A0
#define JOY_PIN_Y A1
#define BTN_PIN_A 4
#define BTN_PIN_B 3
#define BTN_PIN_X 5
#define BTN_PIN_Y 2
#define BTN_PIN_Z 8

#define BTN_PIN_START 6
#define BTN_PIN_SELECT 7

#define MATRIX_SIZE 16

#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)


class Engine {
public:

  // --- Initialization ---
  static void begin() {
    pixels.begin();
    pixels.setBrightness(0);
    pixels.show();
    pinMode(JOY_PIN_X, INPUT);
    pinMode(JOY_PIN_Y, INPUT);

    pinMode(BTN_PIN_A, INPUT_PULLUP);
    pinMode(BTN_PIN_B, INPUT_PULLUP);
    pinMode(BTN_PIN_X, INPUT_PULLUP);
    pinMode(BTN_PIN_Y, INPUT_PULLUP);
    pinMode(BTN_PIN_Z, INPUT_PULLUP);
    pinMode(BTN_PIN_SELECT, INPUT_PULLUP);
    pinMode(BTN_PIN_START, INPUT_PULLUP);

    pinMode(BUZZ_PIN, OUTPUT);
  }

  static void cls() {
    pixels.clear();
    pixels.setBrightness(0);
    fadeValue = 0;
  }

  static void update(int delayInterval) {
    pixels.show();

    updateKeys();

    delay(delayInterval);
    pixels.clear();
    noTone(BUZZ_PIN);
    fadeValue = constrain(fadeValue + 10, 0, 128);
    pixels.setBrightness(fadeValue);
  }

  static void refresh() {
    pixels.show();
  }

  static void updateKeys() {
    // update previous state
    btnPrevStart = btnCurrStart;
    btnPrevSelect = btnCurrSelect;
    btnPrevA = btnCurrA;
    btnPrevB = btnCurrB;
    btnPrevX = btnCurrX;
    btnPrevY = btnCurrY;
    btnPrevZ = btnCurrZ;

    // read current state
    btnCurrStart = !digitalRead(BTN_PIN_START);
    btnCurrSelect = !digitalRead(BTN_PIN_SELECT);
    btnCurrA = !digitalRead(BTN_PIN_A);
    btnCurrB = !digitalRead(BTN_PIN_B);
    btnCurrX = !digitalRead(BTN_PIN_X);
    btnCurrY = !digitalRead(BTN_PIN_Y);
    btnCurrZ = !digitalRead(BTN_PIN_Z);
  }

  static void drawSprite(
    const uint8_t* pixels,
    const uint8_t* palette,
    uint8_t width,
    uint8_t height,
    int offsetX,
    int offsetY,
    bool solid = false
    ) {
    for (uint8_t y = 0; y < height; y++) {
      for (uint8_t x = 0; x < width; x++) {

        uint16_t pixelIndex = y * width + x;
        uint8_t colorIndex = pgm_read_byte(&pixels[pixelIndex]);

        // index 0 = transparent
        if (!solid && colorIndex == 0) continue;

        uint16_t paletteIndex = colorIndex * 3;

        uint8_t r = pgm_read_byte(&palette[paletteIndex + 0]);
        uint8_t g = pgm_read_byte(&palette[paletteIndex + 1]);
        uint8_t b = pgm_read_byte(&palette[paletteIndex + 2]);

        Engine::setPixel(
          x + offsetX,
          y + offsetY,
          Engine::color(r, g, b));
      }
    }
  }

  // --- Pixel Helpers ---
  static void setPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= MATRIX_SIZE || y < 0 || y >= MATRIX_SIZE) return;

    // Rotate 90° clockwise
    int rx = MATRIX_SIZE - 1 - y;
    int ry = x;

    int i = (ry % 2 == 0)
              ? (MATRIX_SIZE - 1 - rx + ry * MATRIX_SIZE)
              : (rx + ry * MATRIX_SIZE);

    pixels.setPixelColor(i, color);
  }

    // --- Draw a filled rectangle ---
  static void drawBox(int x, int y, int w, int h, uint32_t color) {
    for (int iy = 0; iy < h; iy++) {
      for (int ix = 0; ix < w; ix++) {
        setPixel(x + ix, y + iy, color);
      }
    }
  }

  static void drawRect(int x, int y, int w, int h, uint32_t color) {
    for (int ix = 0; ix < w; ix++) {
      setPixel(x + ix, y, color);
      setPixel(x + ix, y + h - 1, color);
    }
    for (int iy = 0; iy < h; iy++) {
      setPixel(x, y + iy, color);
      setPixel(x + w - 1, y + iy, color);
    }
  }

  static void drawCircleFilled(int cx, int cy, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
      for (int x = -r; x <= r; x++) {
        if (x*x + y*y <= r*r)
          setPixel(cx + x, cy + y, color);
      }
    }
  }

  static void drawChecker(int x, int y, int w, int h, uint32_t c1, uint32_t c2) {
    for (int iy = 0; iy < h; iy++) {
      for (int ix = 0; ix < w; ix++) {
        if ((ix + iy) % 2 == 0)
          setPixel(x + ix, y + iy, c1);
        else
          setPixel(x + ix, y + iy, c2);
      }
    }
  }

  static void drawChecker2x(int x, int y, int w, int h, uint32_t c1, uint32_t c2) {
      for (int iy = 0; iy < h; iy += 2) {
          for (int ix = 0; ix < w; ix += 2) {
              uint32_t color = ((ix / 2 + iy / 2) % 2 == 0) ? c1 : c2;
              // fill 2x2 block
              for (int by = 0; by < 2; by++)
                  for (int bx = 0; bx < 2; bx++)
                      setPixel(x + ix + bx, y + iy + by, color);
          }
      }
  }

  static void drawStars(int x, int y, int w, int h,
                      uint8_t density,
                      uint32_t dimColor,
                      uint32_t brightColor)
  {
    for (int iy = 0; iy < h; iy++) {
      for (int ix = 0; ix < w; ix++) {

        uint8_t lx = ix & 15;   // repeat every 16
        uint8_t ly = iy & 15;

        // stronger 2D hash
        uint8_t r = lx * 197 + ly * 101;
        r ^= r >> 3;
        r ^= lx * 29;
        r ^= ly * 71;
        r *= 53;
        r ^= r >> 4;

        if (r < density)
        {
          if (r < (density >> 2))
            setPixel(x + ix, y + iy, brightColor);
          else
            setPixel(x + ix, y + iy, dimColor);
        }
      }
    }
  }

  static void drawBricks(int x, int y, int w, int h, uint32_t brickColor, uint32_t mortarColor) {
      const int brickW = 4;
      const int brickH = 3;

      for (int iy = 0; iy < h; iy++) {
          int brickRow = iy / brickH;
          int rowOffset = (brickRow % 2) ? brickW / 2 : 0;

          for (int ix = 0; ix < w; ix++) {

              int bx = (ix + rowOffset) % brickW;
              int by = iy % brickH;

              bool mortar = (bx == 0) || (by == 0);

              setPixel(x + ix, y + iy, mortar ? mortarColor : brickColor);
          }
      }
  }

  static void drawPattern(int x, int y, int w, int h, const uint8_t* tile, int tileW, int tileH, uint32_t color) {
      for (int iy = 0; iy < h; iy++) {
          for (int ix = 0; ix < w; ix++) {
              int tx = ix % tileW;
              int ty = iy % tileH;
              if (tile[ty] & (1 << (tileW - 1 - tx)))
                  setPixel(x + ix, y + iy, color);
          }
      }
  }

  static void drawBorder(int x, int y, int w, int h, uint32_t color) {
      for (int ix = 0; ix < w; ix++) {
          setPixel(x + ix, y, color);
          setPixel(x + ix, y + h - 1, color);
      }
      for (int iy = 0; iy < h; iy++) {
          setPixel(x, y + iy, color);
          setPixel(x + w - 1, y + iy, color);
      }
  }

  static void drawDigit3x4(int x, int y, int digit, uint32_t color) {
    if (digit < 0 || digit > 9) return;

    for (int row = 0; row < 4; row++) {
      uint8_t rowData = pgm_read_byte(&font3x4[digit][row]);

      for (int col = 0; col < 3; col++) {
        if (rowData & (1 << (2 - col))) {
          setPixel(x + col, y + row, color);
        }
      }
    }
  }

  static void drawNumber3x4(int x, int y, int number, uint32_t color) {
    if (number == 0) {
      drawDigit3x4(x,y,0,color);
      return;
    }

    int digits[6];
    int count = 0;

    while (number > 0) {
      digits[count++] = number % 10;
      number /= 10;
    }

    int offset = 0;
    for (int i = count - 1; i >= 0; i--) {
      drawDigit3x4(x + offset, y, digits[i], color);
      offset += 4; // 3 pixels + 1 space
    }
  }

  static void drawNumber3x4Vertical(int x, int y, int number, uint32_t color) {

    if (number == 0) {
      drawDigit3x4(x, y, 0, color);
      return;
    }

    int digits[6];
    int count = 0;

    while (number > 0) {
      digits[count++] = number % 10;
      number /= 10;
    }

    int offset = 0;

    for (int i = count - 1; i >= 0; i--) {
      drawDigit3x4(x, y + offset, digits[i], color);
      offset += 5; // 4 pixels height + 1 space
    }
  }

  static void drawNumber3x4Right(int x, int y, int number, uint32_t color) {
    if (number == 0) {
      drawDigit3x4(x - 2, y, 0, color); 
      return;
    }

    int digits[6];
    int count = 0;

    while (number > 0) {
      digits[count++] = number % 10;
      number /= 10;
    }

    int totalWidth = count * 4 - 1;
    int startX = x - totalWidth + 1;

    int offset = 0;
    for (int i = count - 1; i >= 0; i--) {
      drawDigit3x4(startX + offset, y, digits[i], color);
      offset += 4;
    }
  }

  static void drawText3x4(int x, int y, const char* text, uint32_t color) {

    while (*text) {
      char c = *text++;
      uint8_t index = 0xFF;

      if (c >= 'A' && c <= 'Z') index = c - 'A';
      else if (c >= 'a' && c <= 'z') index = c - 'a';

      if (index == 0xFF) continue;

      for (uint8_t row = 0; row < 4; row++) {               // outer = row
        uint8_t data = pgm_read_byte(&font3x4abc[index][row]); // each row = byte
        for (uint8_t col = 0; col < 3; col++) {            // inner = column
          if (data & (1 << (2 - col))) {                   // check bit in row
            Engine::setPixel(x + col, y + row, color);
          }
        }
      }

      x += 4; // spacing
    }
  }
  
  static int getAxisX() {
    int val = analogRead(JOY_PIN_X) - 512;

    if (val > deadzone) return 1;
    if (val < -deadzone) return -1;
    return 0;
  }

  static int getAxisY() {
    int val = analogRead(JOY_PIN_Y) - 512;

    if (val > deadzone) return -1;  // Invert if needed
    if (val < -deadzone) return 1;
    return 0;
  }

  // --- GetKeyDown: fires once when button is pressed ---
  static bool getKeyDownStart() {
    return btnCurrStart && !btnPrevStart;
  }
  static bool getKeyDownSelect() {
    return btnCurrSelect && !btnPrevSelect;
  }
  static bool getKeyDownA() {
    return btnCurrA && !btnPrevA;
  }
  static bool getKeyDownB() {
    return btnCurrB && !btnPrevB;
  }
  static bool getKeyDownX() {
    return btnCurrX && !btnPrevX;
  }
  static bool getKeyDownY() {
    return btnCurrY && !btnPrevY;
  }
  static bool getKeyDownZ() {
    return btnCurrZ && !btnPrevZ;
  }

  // --- GetKeyUp: fires once when button is released ---
  static bool getKeyUpStart() {
    return !btnCurrStart && btnPrevStart;
  }
  static bool getKeyUpSelect() {
    return !btnCurrSelect && btnPrevSelect;
  }
  static bool getKeyUpA() {
    return !btnCurrA && btnPrevA;
  }
  static bool getKeyUpB() {
    return !btnCurrB && btnPrevB;
  }
  static bool getKeyUpX() {
    return !btnCurrX && btnPrevX;
  }
  static bool getKeyUpY() {
    return !btnCurrY && btnPrevY;
  }
  static bool getKeyUpZ() {
    return !btnCurrZ && btnPrevZ;
  }

  // --- GetKey: returns current pressed state ---
  static bool getKeyStart() {
    return btnCurrStart;
  }
  static bool getKeySelect() {
    return btnCurrSelect;
  }
  static bool getKeyA() {
    return btnCurrA;
  }
  static bool getKeyB() {
    return btnCurrB;
  }
  static bool getKeyX() {
    return btnCurrX;
  }
  static bool getKeyY() {
    return btnCurrY;
  }
  static bool getKeyZ() {
    return btnCurrZ;
  }

  // --- Sound ---
  static void beep(int duration, int freq = 800) {
    if (!mute) {
      tone(BUZZ_PIN, freq);
    }
    delay(duration);
    noTone(BUZZ_PIN);
  }

  static void toneMapSpeed(int speedDelay) {
    if (!mute) {
      tone(BUZZ_PIN, map(speedDelay, 80, 25, 120, 600));
    }
  }

  // --- Public color helper ---
  static uint32_t color(int r, int g, int b) {
    return pixels.Color(r, g, b);
  }

  inline static bool mute = false;
  inline static int deadzone = 200;

  inline static uint32_t red = color(255, 0, 0);
  inline static uint32_t green = color(0, 255, 0);
  inline static uint32_t blue = color(0, 0, 255);
  inline static uint32_t yellow = color(255, 255, 0);
  inline static uint32_t white = color(255, 255, 255);
  inline static uint32_t gray = color(128, 128, 128);
  inline static uint32_t black = color(0, 0, 0);

private:
  static Adafruit_NeoPixel pixels;
  // --- Button state tracking ---
  inline static bool btnCurrStart = false, btnPrevStart = false;
  inline static bool btnCurrSelect = false, btnPrevSelect = false;
  inline static bool btnCurrA = false, btnPrevA = false;
  inline static bool btnCurrB = false, btnPrevB = false;
  inline static bool btnCurrX = false, btnPrevX = false;
  inline static bool btnCurrY = false, btnPrevY = false;
  inline static bool btnCurrZ = false, btnPrevZ = false;

  inline static uint8_t fadeValue = 0;
  
  inline static const uint8_t font3x4[10][4] PROGMEM = {
    {0b111,0b101,0b101,0b111}, // 0
    {0b110,0b010,0b010,0b111}, // 1
    {0b111,0b011,0b100,0b111}, // 2
    {0b111,0b011,0b001,0b111}, // 3
    {0b101,0b101,0b111,0b001}, // 4
    {0b111,0b110,0b001,0b111}, // 5
    {0b011,0b100,0b111,0b111}, // 6
    {0b111,0b001,0b001,0b001}, // 7
    {0b111,0b101,0b111,0b111}, // 8
    {0b111,0b111,0b001,0b110}, // 9
  };

  inline static const uint8_t font3x4abc[26][4] PROGMEM = {
    {0b010,0b101,0b111,0b101}, // A
    {0b110,0b101,0b110,0b111}, // B
    {0b111,0b100,0b100,0b111}, // C
    {0b110,0b101,0b101,0b110}, // D
    {0b111,0b100,0b110,0b111}, // E
    {0b111,0b100,0b110,0b100}, // F
    {0b011,0b100,0b101,0b011}, // G
    {0b101,0b101,0b111,0b101}, // H
    {0b111,0b010,0b010,0b111}, // I
    {0b001,0b001,0b101,0b010}, // J
    {0b101,0b110,0b100,0b101}, // K
    {0b100,0b100,0b100,0b111}, // L
    {0b101,0b111,0b111,0b101}, // M
    {0b101,0b111,0b111,0b101}, // N
    {0b010,0b101,0b101,0b010}, // O
    {0b110,0b101,0b110,0b100}, // P
    {0b010,0b101,0b111,0b011}, // Q
    {0b110,0b101,0b110,0b101}, // R
    {0b011,0b100,0b010,0b110}, // S
    {0b111,0b010,0b010,0b010}, // T
    {0b101,0b101,0b101,0b111}, // U
    {0b101,0b101,0b010,0b010}, // V
    {0b101,0b111,0b111,0b101}, // W
    {0b101,0b010,0b010,0b101}, // X
    {0b101,0b010,0b010,0b010}, // Y
    {0b111,0b001,0b010,0b111}  // Z
  };
};

// Define the static Adafruit_NeoPixel instance
Adafruit_NeoPixel Engine::pixels(MATRIX_SIZE* MATRIX_SIZE, LED_PIN, NEO_GRB + NEO_KHZ800);
