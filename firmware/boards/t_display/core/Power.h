#pragma once

#include "core/IPower.h"
#include "pins_arduino.h"
#include <Arduino.h>
#include "esp_sleep.h"

class PowerImpl : public IPower
{
public:
  void begin() override {
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);

    pinMode(BAT_ADC_PIN, INPUT);
  }

  uint8_t getBatteryPercentage() override {
    float mv = (float)analogReadMilliVolts(BAT_ADC_PIN) * 2.0f;

    float pct = (mv - 3300.0f) / (4150.0f - 3350.0f) * 100.0f;

    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;

    return (uint8_t)pct;
  }

  bool isCharging() override {
    return false;
  }

  // 🔋 POWER OFF (DEEP SLEEP + WAKE CONFIG)
  void powerOff() override {

    // 🔥 WAKE SOURCE (BTN_UP GPIO 0)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);

    // optional safety: bisa tambah BTN_B kalau mau
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);

    // 🧼 power cleanup ringan
    digitalWrite(ADC_EN, LOW);
    Serial.flush();

    // 💀 sleep
    esp_deep_sleep_start();
  }
};
