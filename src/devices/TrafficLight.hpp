#pragma once
#include <Arduino.h>
#include "core/Types.hpp"

class TrafficLight {
public:
  void begin(uint8_t pinRed, uint8_t pinGreen);
  
  // Control del estado
  void setOccupied(); // Rojo ON, Verde OFF
  void setFree();     // Rojo OFF, Verde ON
  void setOff();      // Ambos OFF (para debugging/mantenimiento)
  
  // Estado actual
  SlotState getState() const { return currentState_; }
  bool isOccupied() const { return currentState_ == SlotState::OCCUPIED; }
  bool isFree() const { return currentState_ == SlotState::FREE; }
  
  // Para debugging
  uint8_t getRedPin() const { return redPin_; }
  uint8_t getGreenPin() const { return greenPin_; }

private:
  uint8_t redPin_{255};
  uint8_t greenPin_{255};
  SlotState currentState_{SlotState::FREE};
  bool initialized_{false};
};