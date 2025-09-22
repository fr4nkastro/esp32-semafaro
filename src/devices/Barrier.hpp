#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "core/Config.hpp"
#include "core/Types.hpp"

class Barrier {
public:
  void begin(uint8_t pwmPin);
  void setAngles(uint8_t closedDeg, uint8_t openDeg);
  
  // Comandos no bloqueantes
  void open();
  void close();
  void stop(); // Detener movimiento inmediatamente
  
  // Update no bloqueante - debe llamarse periódicamente
  void update(uint32_t nowMs, bool safeSensorActive);
  
  // Estado actual
  BarrierState getState() const { return state_; }
  bool isMoving() const { return state_ == BarrierState::OPENING || state_ == BarrierState::CLOSING; }
  bool isOpen() const { return state_ == BarrierState::OPEN; }
  bool isClosed() const { return state_ == BarrierState::CLOSED; }
  bool isFault() const { return state_ == BarrierState::FAULT; }
  
  // Para debugging
  uint8_t getCurrentAngle() const { return currentAngle_; }

private:
  void moveTo(uint8_t targetAngle);
  void setState(BarrierState newState);
  bool hasReachedTarget() const;

  Servo servo_;
  uint8_t pin_{255};
  
  // Configuración de ángulos
  uint8_t openAngle_{Cfg::kServoOpenDeg};
  uint8_t closedAngle_{Cfg::kServoClosedDeg};
  
  // Estado actual
  BarrierState state_{BarrierState::CLOSED};
  uint8_t currentAngle_{Cfg::kServoClosedDeg};
  uint8_t targetAngle_{Cfg::kServoClosedDeg};
  
  // Control de movimiento suave
  uint32_t lastStepMs_{0};
  uint32_t commandStartMs_{0};
  
  // Timeouts
  const uint32_t stepIntervalMs_ = Cfg::kBarrierStepMs;
  const uint32_t openTimeoutMs_ = Cfg::kOpenTimeout;
  const uint32_t closeTimeoutMs_ = Cfg::kCloseTimeout;
  
  // Para debugging/logging
  BarrierState lastLoggedState_{BarrierState::CLOSED};
};