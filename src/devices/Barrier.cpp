#include "Barrier.hpp"
#include "core/Logger.hpp"

void Barrier::begin(uint8_t pwmPin) {
  pin_ = pwmPin;
  
  // Configurar servo
  servo_.attach(pin_);
  
  // Posición inicial cerrada
  currentAngle_ = closedAngle_;
  targetAngle_ = closedAngle_;
  servo_.write(currentAngle_);
  
  state_ = BarrierState::CLOSED;
  lastLoggedState_ = state_;
  commandStartMs_ = millis();
  lastStepMs_ = millis();
  
  LOG_INFO("Barrier initialized on pin %d (closed: %d°, open: %d°)", 
           pin_, closedAngle_, openAngle_);
}

void Barrier::setAngles(uint8_t closedDeg, uint8_t openDeg) {
  closedAngle_ = closedDeg;
  openAngle_ = openDeg;
  
  // Si estamos cerrados, actualizar posición actual
  if (state_ == BarrierState::CLOSED) {
    currentAngle_ = closedAngle_;
    targetAngle_ = closedAngle_;
    servo_.write(currentAngle_);
  }
  
  LOG_INFO("Barrier angles updated (closed: %d°, open: %d°)", closedDeg, openDeg);
}

void Barrier::open() {
  if (state_ == BarrierState::FAULT) {
    LOG_WARN("Cannot open barrier - in FAULT state");
    return;
  }
  
  if (state_ != BarrierState::OPEN && state_ != BarrierState::OPENING) {
    targetAngle_ = openAngle_;
    commandStartMs_ = millis();
    setState(BarrierState::OPENING);
    LOG_INFO("Barrier opening command issued");
  }
}

void Barrier::close() {
  if (state_ == BarrierState::FAULT) {
    LOG_WARN("Cannot close barrier - in FAULT state");
    return;
  }
  
  if (state_ != BarrierState::CLOSED && state_ != BarrierState::CLOSING) {
    targetAngle_ = closedAngle_;
    commandStartMs_ = millis();
    setState(BarrierState::CLOSING);
    LOG_INFO("Barrier closing command issued");
  }
}

void Barrier::stop() {
  targetAngle_ = currentAngle_;
  
  if (isMoving()) {
    // Determinar estado final basado en posición
    if (abs(currentAngle_ - openAngle_) < abs(currentAngle_ - closedAngle_)) {
      setState(BarrierState::OPEN);
    } else {
      setState(BarrierState::CLOSED);
    }
    LOG_INFO("Barrier stopped at %d°", currentAngle_);
  }
}

void Barrier::update(uint32_t nowMs, bool safeSensorActive) {
  // Si el sensor de seguridad está activo y estamos cerrando, detener
  if (safeSensorActive && state_ == BarrierState::CLOSING) {
    LOG_WARN("Safety sensor active - stopping barrier closure");
    stop();
    return;
  }
  
  // Verificar timeouts
  uint32_t elapsed = nowMs - commandStartMs_;
  if (state_ == BarrierState::OPENING && elapsed > openTimeoutMs_) {
    LOG_ERR("Barrier open timeout (%lu ms)", elapsed);
    setState(BarrierState::FAULT);
    return;
  }
  
  if (state_ == BarrierState::CLOSING && elapsed > closeTimeoutMs_) {
    LOG_ERR("Barrier close timeout (%lu ms)", elapsed);
    setState(BarrierState::FAULT);
    return;
  }
  
  // Movimiento suave paso a paso
  if (isMoving() && (nowMs - lastStepMs_) >= stepIntervalMs_) {
    lastStepMs_ = nowMs;
    
    if (currentAngle_ != targetAngle_) {
      // Calcular siguiente paso
      if (currentAngle_ < targetAngle_) {
        currentAngle_ = min(static_cast<uint8_t>(currentAngle_ + Cfg::kServoStepDeg), targetAngle_);
      } else {
        currentAngle_ = max(static_cast<uint8_t>(currentAngle_ - Cfg::kServoStepDeg), targetAngle_);
      }
      
      // Aplicar posición
      servo_.write(currentAngle_);
      
      // Verificar si llegamos al destino
      if (hasReachedTarget()) {
        if (state_ == BarrierState::OPENING) {
          setState(BarrierState::OPEN);
          LOG_INFO("Barrier fully opened");
        } else if (state_ == BarrierState::CLOSING) {
          setState(BarrierState::CLOSED);
          LOG_INFO("Barrier fully closed");
        }
      }
    }
  }
}

void Barrier::setState(BarrierState newState) {
  if (state_ != newState) {
    state_ = newState;
    
    // Log solo cambios de estado
    if (lastLoggedState_ != newState) {
      LOG_DEBUG("Barrier state: %d -> %d", (int)lastLoggedState_, (int)newState);
      lastLoggedState_ = newState;
    }
  }
}

bool Barrier::hasReachedTarget() const {
  return currentAngle_ == targetAngle_;
}