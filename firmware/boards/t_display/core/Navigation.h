// LilyGO T-Display 16MB — two-button navigation.
// BTN_UP (GPIO 35): UP
// BTN_B  (GPIO 0): single click = DOWN, double click = PRESS, hold = BACK.
#pragma once

#include "core/INavigation.h"

class NavigationImpl : public INavigation
{
public:
  void begin() override {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_B, INPUT_PULLUP);
    _rawBottom = digitalRead(BTN_B) == LOW;
    _stableBottom = _rawBottom;
    _rawChangedAt = millis();
  }

  void update() override {
    const uint32_t now = millis();

    // UP is a direct held direction. Cancel any pending bottom-button click
    // so a simultaneous/overlapping UP press cannot later trigger DOWN/SELECT.
    if (digitalRead(BTN_UP) == LOW) {
      cancelBottomClick();
      updateState(DIR_UP);
      return;
    }

    // Debounce the bottom button before interpreting edges.
    const bool rawBottom = digitalRead(BTN_B) == LOW;
    if (rawBottom != _rawBottom) {
      _rawBottom = rawBottom;
      _rawChangedAt = now;
    }

    if ((now - _rawChangedAt) >= DEBOUNCE_MS && _stableBottom != _rawBottom) {
      _stableBottom = _rawBottom;
      if (_stableBottom) onBottomPressed(now);
      else onBottomReleased(now);
    }

    if (_stableBottom) {
      if (!_longPressFired && (now - _pressStartedAt) >= LONG_PRESS_MS) {
        _longPressFired = true;
        _pendingSingle = false;
        _doublePress = false;
        updateState(DIR_BACK);
      } else if (_longPressFired) {
        updateState(DIR_BACK);
      } else if (_doublePress) {
        updateState(DIR_PRESS);
      } else {
        updateState(DIR_NONE);
      }
      return;
    }

    // Resolve a single click only after the double-click window has elapsed.
    if (_pendingSingle && (now - _releasedAt) >= DBL_CLICK_MS) {
      _pendingSingle = false;
      _emitTap = true;
    }

    if (_emitPress) {
      _emitPress = false;
      updateState(DIR_PRESS);
    } else if (_emitTap) {
      _emitTap = false;
      updateState(DIR_DOWN);
    } else {
      updateState(DIR_NONE);
    }
  }

private:
  static constexpr uint32_t DEBOUNCE_MS = 25;
  static constexpr uint32_t DBL_CLICK_MS = 280;
  static constexpr uint32_t LONG_PRESS_MS = 600;

  void onBottomPressed(uint32_t now) {
    _pressStartedAt = now;
    _longPressFired = false;
    _doublePress = false;

    if (_pendingSingle && (now - _releasedAt) <= DBL_CLICK_MS) {
      _pendingSingle = false;
      _doublePress = true;
    } else {
      // A prior click whose window expired is a single click. Normally it is
      // emitted by update(); this guard handles a delayed polling edge safely.
      if (_pendingSingle) _emitTap = true;
      _pendingSingle = false;
    }
  }

  void onBottomReleased(uint32_t now) {
    if (_longPressFired) {
      _longPressFired = false;
      _doublePress = false;
      return;
    }

    if (_doublePress) {
      _doublePress = false;
      _emitPress = true;
      return;
    }

    _pendingSingle = true;
    _releasedAt = now;
  }

  void cancelBottomClick() {
    _pendingSingle = false;
    _emitTap = false;
    _emitPress = false;
    _doublePress = false;
    _longPressFired = false;
  }

  uint32_t _rawChangedAt = 0;
  uint32_t _pressStartedAt = 0;
  uint32_t _releasedAt = 0;
  bool _rawBottom = false;
  bool _stableBottom = false;
  bool _pendingSingle = false;
  bool _doublePress = false;
  bool _longPressFired = false;
  bool _emitTap = false;
  bool _emitPress = false;
};
