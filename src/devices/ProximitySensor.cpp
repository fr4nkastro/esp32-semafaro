#include "ProximitySensor.hpp"
#include "core/Logger.hpp"

void ProximitySensor::begin(uint8_t pin, bool pullup, bool normallyHigh) {
  pin_ = pin;
  pullup_ = pullup;
  normallyHigh_ = normallyHigh;
  
  pinMode(pin_, pullup_ ? INPUT_PULLUP : INPUT);
  
  // Leer estado inicial
  lastRaw_ = digitalRead(pin_) == HIGH;
  stable_ = lastRaw_;
  lastStable_ = stable_;
  lastChange_ = millis();
  
  LOG_INFO("ProximitySensor initialized on pin %d (pullup: %d, normallyHigh: %d)", 
           pin_, pullup_, normallyHigh_);
}

bool ProximitySensor::isDetected(uint32_t nowMs) {
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
  
  // Interpretar según tipo de sensor:
  // PNP (normallyHigh=true): HIGH cuando detecta
  // NPN (normallyHigh=false): LOW cuando detecta
  bool detected = normallyHigh_ ? stable_ : !stable_;
  
  return detected;
}

bool ProximitySensor::wasActivated() {
  bool currentDetected = normallyHigh_ ? stable_ : !stable_;
  bool lastDetected = normallyHigh_ ? lastStable_ : !lastStable_;
  
  // Detectar flanco de activación
  bool edge = currentDetected && !lastDetected;
  lastStable_ = stable_;
  
  return edge;
}

bool ProximitySensor::wasDeactivated() {
  bool currentDetected = normallyHigh_ ? stable_ : !stable_;
  bool lastDetected = normallyHigh_ ? lastStable_ : !lastStable_;
  
  // Detectar flanco de desactivación
  bool edge = !currentDetected && lastDetected;
  lastStable_ = stable_;
  
  return edge;
}