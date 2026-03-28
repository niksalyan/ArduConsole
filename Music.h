#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>

class Music {
public:

  // --- INIT ---
  static void begin(uint8_t buzzPin) {
    _buzzPin = buzzPin;
    pinMode(_buzzPin, OUTPUT);
  }

  // --- PLAY (PROGMEM) ---
  static void play(const char* progmemSong, bool loop = false) {
    _song = progmemSong;
    _index = 0;
    _frameCounter = 0;
    _octaveShift = 0;
    _sharp = false;
    _currentNote = 'C';
    _playing = true;
    _loop = loop;
  }

  static void stop() {
    noTone(_buzzPin);
    _playing = false;
  }

  static bool isPlaying() {
    return _playing;
  }

  // --- UPDATE ---
  static void update() {
    if (!_playing) return;

    if (_frameCounter > 0) {
      _frameCounter--;
      return;
    }

    parseNext();
  }

private:

  // --- STATE ---
  inline static uint8_t _buzzPin = 0;

  inline static const char* _song = nullptr;
  inline static int _index = 0;
  inline static int _frameCounter = 0;

  inline static int _octaveShift = 0;
  inline static bool _sharp = false;
  inline static bool _playing = false;
  inline static bool _loop = false;

  inline static char _currentNote = 'C';

  // --- READ FROM PROGMEM ---
  static char readChar(int index) {
    return pgm_read_byte(&_song[index]);
  }

  // --- PARSER ---
  static void parseNext() {

    while (true) {

      char c = readChar(_index++);

      // END OF STRING
      if (c == '\0') {
        if (_loop) {
          _index = 0;
          continue;
        } else {
          stop();
          return;
        }
      }

      // skip spaces
      if (c == ' ') continue;

      // NOTE
      if (isNote(c)) {
        _currentNote = c;
        continue;
      }

      // SHARP
      if (c == '#') {
        _sharp = true;
        continue;
      }

      // OCTAVE
      if (c == '+') {
        _octaveShift++;
        continue;
      }

      if (c == '-') {
        _octaveShift--;
        continue;
      }

      // REST
      if (c == 'R') {
        _currentNote = 'R';
        continue;
      }

      // DURATION
      if (isdigit(c)) {
        int duration = c - '0';
        if (duration <= 0) duration = 1;

        playCurrent(duration);
        return;
      }
    }
  }

  // --- PLAY ---
  static void playCurrent(int duration) {

    _frameCounter = duration;

    if (_currentNote == 'R') {
      noTone(_buzzPin);
      _sharp = false;
      return;
    }

    int freq = getFrequency(_currentNote, 4 + _octaveShift);

    if (_sharp) {
      freq = (int)(freq * 1.05946f);
    }

    tone(_buzzPin, freq);

    _sharp = false;
  }

  // --- HELPERS ---
  static bool isNote(char c) {
    return (c >= 'A' && c <= 'G');
  }

  static int getFrequency(char note, int octave) {

    int base[] = {261, 293, 329, 349, 392, 440, 493};
    const char* names = "CDEFGAB";

    for (int i = 0; i < 7; i++) {
      if (names[i] == note) {

        int freq = base[i];
        int diff = octave - 4;

        while (diff > 0) { freq *= 2; diff--; }
        while (diff < 0) { freq /= 2; diff++; }

        return freq;
      }
    }

    return 0;
  }
};