#pragma once
#include <Arduino.h>
#include "core/Config.hpp"

class ProximitySensor {
public:
  void begin(uint8_t pin, bool pullup = true, bool normallyHigh = true);
  bool isDetected(uint32_t nowMs);
  bool wasActivated(); // Edge detection - true cuando detecta presencia
  bool wasDeactivated(); // Edge detection - true cuando deja de detectar
  
private:
  uint8_t pin_{255};
  bool pullup_{true};
  bool normallyHigh_{true}; // true para sensores PNP, false para NPN
  bool lastRaw_{false};
  bool stable_{false};
  bool lastStable_{false};
  uint32_t lastChange_{0};
  uint16_t debounceMs_{Cfg::kSensDebounceMs};
};