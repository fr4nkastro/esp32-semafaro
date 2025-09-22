#include "Button.hpp"
#include "core/Logger.hpp"

void Button::begin(uint8_t pin, bool pullup, bool activeLow) {
  pin_ = pin;
  pullup_ = pullup;
  activeLow_ = activeLow;
  
  pinMode(pin_, pullup_ ? INPUT_PULLUP : INPUT);
  
  // Leer estado inicial
  lastRaw_ = digitalRead(pin_) == HIGH;
  stable_ = lastRaw_;
  lastStable_ = stable_;
  lastChange_ = millis();
  
  LOG_INFO("Button initialized on pin %d (pullup: %d, activeLow: %d)", 
           pin_, pullup_, activeLow_);
}

bool Button::isPressed(uint32_t nowMs) {
  // Leer estado raw
  bool raw = digitalRead(pin_) == HIGH;
  
  // Detectar cambios
  if (raw != lastRaw_) {
    lastChange_ = nowMs;
    lastRaw_ = raw;
  }
  
  // Aplicar debounce
  if ((nowMs - lastChange_) > debounceMs_) {
    stable_ = raw;
  }
  
  // Aplicar lógica activo-bajo si es necesario
  bool pressed = activeLow_ ? !stable_ : stable_;
  
  return pressed;
}

bool Button::wasPressed() {
  bool currentStable = stable_;
  
  // Detectar flanco de subida en el estado estable
  bool edge = currentStable && !lastStable_;
  lastStable_ = currentStable;
  
  // Para activeLow, detectar flanco de bajada
  if (activeLow_) {
    edge = !currentStable && lastStable_;
  }
  
  return edge;
}