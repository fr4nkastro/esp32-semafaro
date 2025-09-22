#pragma once
#include "IPlatform.hpp"
#include <unordered_map>
#include <chrono>

#ifdef NATIVE_PLATFORM

// Native/PC platform implementation with mocks
class NativePlatform : public IPlatform {
public:
  NativePlatform() {
    startTime_ = std::chrono::steady_clock::now();
  }
  
  void pinMode(uint8_t pin, uint8_t mode) override {
    pinModes_[pin] = mode;
  }
  
  void digitalWrite(uint8_t pin, uint8_t value) override {
    if (pinModes_[pin] == OUTPUT) {
      pinStates_[pin] = (value != 0);
    }
  }
  
  int digitalRead(uint8_t pin) override {
    // Return simulated pin state
    auto it = pinStates_.find(pin);
    return (it != pinStates_.end()) ? (it->second ? HIGH : LOW) : LOW;
  }
  
  uint32_t millis() override {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_);
    return static_cast<uint32_t>(duration.count());
  }
  
  // Test utilities - not part of IPlatform interface
  void setPin(uint8_t pin, bool state) {
    pinStates_[pin] = state;
  }
  
  bool getPin(uint8_t pin) const {
    auto it = pinStates_.find(pin);
    return (it != pinStates_.end()) ? it->second : false;
  }
  
  uint8_t getPinMode(uint8_t pin) const {
    auto it = pinModes_.find(pin);
    return (it != pinModes_.end()) ? it->second : INPUT;
  }
  
  void reset() {
    pinStates_.clear();
    pinModes_.clear();
    startTime_ = std::chrono::steady_clock::now();
  }

private:
  std::unordered_map<uint8_t, bool> pinStates_;
  std::unordered_map<uint8_t, uint8_t> pinModes_;
  std::chrono::steady_clock::time_point startTime_;
};

#endif // NATIVE_PLATFORM