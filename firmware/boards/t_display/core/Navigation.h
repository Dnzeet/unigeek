#pragma once

#include "core/INavigation.h"

class NavigationImpl : public INavigation
{
public:
  void begin() override {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_B,  INPUT_PULLUP);
  }

  void update() override
  {
    bool btnUp   = (digitalRead(BTN_UP) == LOW);
    bool btnDown = (digitalRead(BTN_B)  == LOW);
    uint32_t now = millis();

    // ─────────────────────────────────────────────
    // 🔼 BTN_UP : UP + DOUBLE CLICK = POWER OFF
    // ─────────────────────────────────────────────
    if (btnUp && !_btnUpWasDown) {
      _btnUpWasDown = true;

      if (_waitUpDbl && (now - _lastUpTime) <= DBL_CLICK_MS) {
        _waitUpDbl = false;

        // 🔴 trigger sleep event
        updateState(DIR_NONE);
        updateState(DIR_POWER_OFF);
        return;
      } 
      else {
        _waitUpDbl = true;
        _lastUpTime = now;
        updateState(DIR_UP);
        return;
      }
    }
    else if (!btnUp) {
      _btnUpWasDown = false;

      if (_waitUpDbl && (now - _lastUpTime) > DBL_CLICK_MS) {
        _waitUpDbl = false;
      }
    }

    // ─────────────────────────────────────────────
    // 🔽 BTN_B : DOWN / PRESS / BACK (UNCHANGED)
    // ─────────────────────────────────────────────
    if (_syntheticDown) {
      _syntheticDown = false;
      updateState(DIR_NONE);
      return;
    }

    if (btnDown && !_btnBWasDown) {
      _btnBWasDown = true;

      if (_waitDblClick && (now - _lastDownTime) <= DBL_CLICK_MS) {
        _waitDblClick = false;
        _heldDir = DIR_PRESS;
        updateState(DIR_PRESS);
      } else {
        _waitDblClick = true;
        _lastDownTime = now;
        _heldDir = DIR_NONE;
        updateState(DIR_NONE);
      }
    }
    else if (btnDown) {
      uint32_t held = now - _lastDownTime;

      if (!_btnBLong && held > LONG_PRESS_MS) {
        _btnBLong = true;
        _waitDblClick = false;
        updateState(DIR_BACK);
        return;
      }

      if (_btnBLong) {
        updateState(DIR_NONE);
        return;
      }

      if (_waitDblClick) {
        if (held > DBL_CLICK_MS) {
          _waitDblClick = false;
          _heldDir = DIR_DOWN;
          updateState(DIR_DOWN);
        } else {
          updateState(DIR_NONE);
        }
      } else {
        updateState(_heldDir);
      }
    }
    else {
      _btnBWasDown = false;
      _heldDir = DIR_NONE;

      if (_btnBLong) {
        _btnBLong = false;
        updateState(DIR_NONE);
        return;
      }

      if (_waitDblClick) {
        if ((now - _lastDownTime) > DBL_CLICK_MS) {
          _waitDblClick = false;
          _syntheticDown = true;
          updateState(DIR_DOWN);
          return;
        } else {
          updateState(DIR_NONE);
        }
      } else {
        updateState(DIR_NONE);
      }
    }
  }

private:
  static constexpr uint32_t DBL_CLICK_MS  = 250;
  static constexpr uint32_t LONG_PRESS_MS = 600;

  // 🔼 BTN_UP state
  uint32_t _lastUpTime = 0;
  bool _btnUpWasDown = false;
  bool _waitUpDbl = false;

  // 🔽 BTN_B state
  uint32_t _lastDownTime = 0;
  bool _btnBWasDown = false;
  bool _waitDblClick = false;
  bool _syntheticDown = false;
  bool _btnBLong = false;
  Direction _heldDir = DIR_NONE;
};
