#pragma once
#include "IPlatform.hpp"

#ifdef ESP32_PLATFORM
#include <Arduino.h>

// Arduino/ESP32 platform implementation
class ArduinoPlatform : public IPlatform {
public:
  void pinMode(uint8_t pin, uint8_t mode) override {
    ::pinMode(pin, mode);
  }
  
  void digitalWrite(uint8_t pin, uint8_t value) override {
    ::digitalWrite(pin, value);
  }
  
  int digitalRead(uint8_t pin) override {
    return ::digitalRead(pin);
  }
  
  uint32_t millis() override {
    return ::millis();
  }
};

#endif // ESP32_PLATFORM