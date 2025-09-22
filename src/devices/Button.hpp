#pragma once
#include <Arduino.h>
#include "core/Config.hpp"

class Button {
public:
  void begin(uint8_t pin, bool pullup = true, bool activeLow = true);
  bool isPressed(uint32_t nowMs);
  bool wasPressed(); // Edge detection - true solo una vez por presión

private:
  uint8_t pin_{255};
  bool pullup_{true};
  bool activeLow_{true};
  bool lastRaw_{false};
  bool stable_{false};
  bool lastStable_{false};
  bool edgeDetected_{false};
  uint32_t lastChange_{0};
  const uint16_t debounceMs_ = Cfg::kBtnDebounceMs;
};