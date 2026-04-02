#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>

class Music {
public:

  static const uint8_t CHANNEL_COUNT = 3;

  // --- INIT ---
  static void begin() {}

  // --- PLAY PER CHANNEL ---
  static void play(uint8_t ch, const char* song, bool loop = false) {
    Channel& c = _ch[ch];

    c.song = song;
    c.index = 0;
    c.frameCounter = 0;
    c.octaveShift = 0;
    c.sharp = 0;
    c.currentNote = 'C';
    c.playing = 1;
    c.loop = loop;
    c.frequency = 0;
  }

  static void stop(uint8_t ch) {
    _ch[ch].playing = 0;
    _ch[ch].frequency = 0;
  }

  static uint16_t getFrequency(uint8_t ch) {
    return _ch[ch].frequency;
  }

  static uint8_t getChannelCount() {
    return CHANNEL_COUNT;
  }

  // --- UPDATE (call per frame) ---
  static void update(int buzzPin, int delayInterval) {
    for (uint8_t i = 0; i < CHANNEL_COUNT; i++) {
      updateChannel(_ch[i]);
    }

    if (CHANNEL_COUNT > 0) {
      for (int dl = 0; dl < delayInterval; dl += 10) {
        uint16_t freq = getFrequency(dl % CHANNEL_COUNT);
        if (freq > 0) {
          tone(buzzPin, freq);
        } else {
          noTone(buzzPin);
        }

        delay(10);
      }
    } else {
      delay(delayInterval);
    }
  }

private:

  struct Channel {
    const char* song;
    uint16_t index;

    uint8_t frameCounter;

    int8_t octaveShift;
    uint8_t sharp;
    uint8_t playing;
    uint8_t loop;

    char currentNote;
    uint16_t frequency;
  };

  inline static Channel _ch[CHANNEL_COUNT];

  // --- TABLE ---
  static const uint16_t baseFreq[7] PROGMEM;

  // --- READ ---
  static char readChar(const char* s, uint16_t i) {
    return pgm_read_byte(&s[i]);
  }

  // --- UPDATE CHANNEL ---
  static void updateChannel(Channel& c) {

    if (!c.playing) return;

    if (c.frameCounter) {
      c.frameCounter--;
      return;
    }

    parseNext(c);
  }

  // --- PARSER ---
  static void parseNext(Channel& c) {

    while (true) {

      char ch = readChar(c.song, c.index++);

      if (!ch) {
        if (c.loop) {
          c.index = 0;
          continue;
        }
        c.playing = 0;
        c.frequency = 0;
        return;
      }

      // RESET OCTAVE
      if (ch == ' ') {
        c.octaveShift = 0;
        continue;
      }

      // NOTE
      if (ch >= 'A' && ch <= 'G') {
        c.currentNote = ch;
        continue;
      }

      // SHARP
      if (ch == '#') {
        c.sharp = 1;
        continue;
      }

      // OCTAVE
      if (ch == '+') {
        c.octaveShift++;
        continue;
      }

      if (ch == '-') {
        c.octaveShift--;
        continue;
      }

      // REST
      if (ch == 'R') {
        c.currentNote = 'R';
        continue;
      }

      // DURATION
      if (ch >= '0' && ch <= '9') {
        uint8_t d = ch - '0';
        if (!d) d = 1;

        playCurrent(c, d);
        return;
      }
    }
  }

  // --- PLAY (SET FREQ ONLY) ---
  static void playCurrent(Channel& c, uint8_t duration) {

    c.frameCounter = duration;

    if (c.currentNote == 'R') {
      c.frequency = 0;
      c.sharp = 0;
      return;
    }

    int freq = getFreq(c.currentNote, 4 + c.octaveShift);

    if (c.sharp) {
      freq = (freq * 106) / 100;
    }

    c.frequency = freq;
    c.sharp = 0;
  }

  // --- FREQ ---
  static int getFreq(char note, int8_t octave) {

    uint8_t idx = note - 'C';
    uint16_t freq = pgm_read_word(&baseFreq[idx]);

    int8_t diff = octave - 4;

    if (diff > 0) freq <<= diff;
    else if (diff < 0) freq >>= (-diff);

    return freq;
  }
};

// --- TABLE ---
const uint16_t Music::baseFreq[7] PROGMEM = {
  261, 293, 329, 349, 392, 440, 493
};