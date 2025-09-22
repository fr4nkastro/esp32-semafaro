#pragma once
#include <stdint.h>

// Platform abstraction interface for GPIO and timing operations
class IPlatform {
public:
  // GPIO operations
  virtual void pinMode(uint8_t pin, uint8_t mode) = 0;
  virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;
  virtual int digitalRead(uint8_t pin) = 0;
  
  // Timing operations
  virtual uint32_t millis() = 0;
  
  // Constants that match Arduino definitions
  static constexpr uint8_t HIGH = 1;
  static constexpr uint8_t LOW = 0;
  static constexpr uint8_t INPUT = 0;
  static constexpr uint8_t OUTPUT = 1;
  static constexpr uint8_t INPUT_PULLUP = 2;
  
  virtual ~IPlatform() = default;
};

// Global platform instance (to be initialized per platform)
extern IPlatform* g_platform;