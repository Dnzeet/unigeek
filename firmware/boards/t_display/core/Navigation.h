//
// T-Display 16MB — 2 tombol, logika disamakan dengan Bruce (interface.cpp TTGO):
//
//   GPIO 0  (BTN_UP) : klik = PRESS (select)   | tahan = DOWN berulang (next)
//   GPIO 35 (BTN_B)  : klik = BACK (esc)       | tahan = UP berulang (prev)
//                      klik 2x = power off
//
// ListScreen & kawan-kawan hanya membaca event saat tombol dilepas
// (wasPressed/readDirection), jadi tiap aksi dikirim sebagai "pulse":
// updateState(dir) pada satu tick, lalu updateState(DIR_NONE) di tick berikutnya.
//

#pragma once

#include "core/INavigation.h"
#include "core/Device.h"

class NavigationImpl : public INavigation
{
public:
  void begin() override {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_B,  INPUT_PULLUP);
  }

  void update() override
  {
    uint32_t now = millis();

    // Akhiri pulse dari tick sebelumnya -> event release terkirim ke screen
    if (_pulseOut) {
      _pulseOut = false;
      updateState(DIR_NONE);
      return;
    }

    bool selDown  = (digitalRead(BTN_UP) == LOW);   // GPIO 0
    bool backDown = (digitalRead(BTN_B)  == LOW);   // GPIO 35

    // ── GPIO 0: klik = PRESS, tahan = DOWN berulang ─────────────────────
    if (selDown) {
      if (!_selWasDown) {
        _selWasDown = true;
        _selHold    = false;
        _selStart   = now;
      } else if (!_selHold && (now - _selStart) >= HOLD_MS) {
        _selHold   = true;
        _selRepeat = now - REPEAT_MS;           // pulse pertama langsung
      }

      if (_selHold && (now - _selRepeat) >= REPEAT_MS) {
        _selRepeat = now;
        pulse(DIR_DOWN);
        return;
      }
    } else if (_selWasDown) {
      _selWasDown = false;
      bool wasHold = _selHold;
      _selHold = false;
      if (!wasHold && (now - _selStart) >= DEBOUNCE_MS) {
        pulse(DIR_PRESS);
        return;
      }
    }

    // ── GPIO 35: klik = BACK, tahan = UP berulang, klik 2x = power off ──
    if (backDown) {
      if (!_backWasDown) {
        _backWasDown = true;
        _backHold    = false;
        _backStart   = now;
      } else if (!_backHold && (now - _backStart) >= HOLD_MS) {
        _backHold    = true;
        _waitDbl     = false;                   // tahan membatalkan klik tertunda
        _backRepeat  = now - REPEAT_MS;
      }

      if (_backHold && (now - _backRepeat) >= REPEAT_MS) {
        _backRepeat = now;
        pulse(DIR_UP);
        return;
      }
    } else if (_backWasDown) {
      _backWasDown = false;
      bool wasHold = _backHold;
      _backHold = false;

      if (!wasHold && (now - _backStart) >= DEBOUNCE_MS) {
        if (_waitDbl && (now - _backClickTime) <= DBL_CLICK_MS) {
          _waitDbl = false;
          Uni.Power.powerOff();                 // klik 2x
          return;
        }
        _waitDbl       = true;
        _backClickTime = now;
      }
    }

    // Klik tunggal GPIO 35 dikonfirmasi setelah jendela double-click habis
    if (_waitDbl && !backDown && (now - _backClickTime) > DBL_CLICK_MS) {
      _waitDbl = false;
      pulse(DIR_BACK);
      return;
    }

    updateState(DIR_NONE);
  }

private:
  // Nilai sama dengan Bruce: long_press_time = 200, hold repeat = 300 ms
  static constexpr uint32_t HOLD_MS      = 200;
  static constexpr uint32_t REPEAT_MS    = 300;
  static constexpr uint32_t DBL_CLICK_MS = 250;
  static constexpr uint32_t DEBOUNCE_MS  = 30;

  void pulse(Direction d) {
    updateState(d);
    _pulseOut = true;
  }

  bool     _pulseOut      = false;

  bool     _selWasDown    = false;
  bool     _selHold       = false;
  uint32_t _selStart      = 0;
  uint32_t _selRepeat     = 0;

  bool     _backWasDown   = false;
  bool     _backHold      = false;
  bool     _waitDbl       = false;
  uint32_t _backStart     = 0;
  uint32_t _backRepeat    = 0;
  uint32_t _backClickTime = 0;
};
