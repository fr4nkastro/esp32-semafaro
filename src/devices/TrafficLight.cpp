#include "TrafficLight.hpp"
#include "core/Logger.hpp"

void TrafficLight::begin(uint8_t pinRed, uint8_t pinGreen) {
  redPin_ = pinRed;
  greenPin_ = pinGreen;
  
  // Configurar pines como salida
  pinMode(redPin_, OUTPUT);
  pinMode(greenPin_, OUTPUT);
  
  // Estado inicial: libre (verde encendido)
  setFree();
  initialized_ = true;
  
  LOG_INFO("TrafficLight initialized (Red: %d, Green: %d)", redPin_, greenPin_);
}

void TrafficLight::setOccupied() {
  if (!initialized_) {
    LOG_ERR("TrafficLight not initialized");
    return;
  }
  
  digitalWrite(redPin_, HIGH);
  digitalWrite(greenPin_, LOW);
  currentState_ = SlotState::OCCUPIED;
  
  LOG_DEBUG("TrafficLight set to OCCUPIED (pins %d/%d)", redPin_, greenPin_);
}

void TrafficLight::setFree() {
  if (!initialized_) {
    LOG_ERR("TrafficLight not initialized");
    return;
  }
  
  digitalWrite(redPin_, LOW);
  digitalWrite(greenPin_, HIGH);
  currentState_ = SlotState::FREE;
  
  LOG_DEBUG("TrafficLight set to FREE (pins %d/%d)", redPin_, greenPin_);
}

void TrafficLight::setOff() {
  if (!initialized_) {
    LOG_ERR("TrafficLight not initialized");
    return;
  }
  
  digitalWrite(redPin_, LOW);
  digitalWrite(greenPin_, LOW);
  // Mantener el estado lógico, solo apagar físicamente
  
  LOG_DEBUG("TrafficLight set to OFF (pins %d/%d)", redPin_, greenPin_);
}