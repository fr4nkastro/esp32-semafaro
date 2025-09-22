#include "AccessController.hpp"
#include "core/Logger.hpp"

void AccessController::begin(Barrier* barrier, SlotManager* slots,
                            Button* btnVip, Button* btnCarga, Button* btnReg, Button* btnExit,
                            ProximitySensor* safe) {
  barrier_ = barrier;
  slots_ = slots;
  btnVip_ = btnVip;
  btnCarga_ = btnCarga;
  btnReg_ = btnReg;
  btnExit_ = btnExit;
  safe_ = safe;
  
  state_ = State::IDLE;
  lastLoggedState_ = state_;
  stateStartMs_ = millis();
  assignedSlot_ = -1;
  isExitOperation_ = false;
  safeSensorLastState_ = false;
  
  initialized_ = true;
  
  LOG_INFO("AccessController initialized in IDLE state");
}

void AccessController::update(uint32_t nowMs) {
  if (!initialized_) return;

  // Leer estado del sensor de seguridad (con anti-spam logging)
  bool safeSensorActive = safe_->isDetected(nowMs);
  if (safeSensorActive != safeSensorLastState_) {
    LOG_INFO("Safety sensor: %s", safeSensorActive ? "ACTIVE" : "INACTIVE");
    safeSensorLastState_ = safeSensorActive;
  }

  // Ejecutar handler del estado actual
  switch (state_) {
    case State::IDLE:
      handleIdle(nowMs);
      break;
    case State::CHECK_CAPACITY:
      handleCheckCapacity(nowMs);
      break;
    case State::OPENING:
      handleOpening(nowMs);
      break;
    case State::WAIT_PASS:
      handleWaitPass(nowMs);
      break;
    case State::CLOSING:
      handleClosing(nowMs);
      break;
    case State::FAULT:
      handleFault(nowMs);
      break;
  }
}

void AccessController::reset() {
  if (state_ == State::FAULT) {
    LOG_INFO("Manual reset from FAULT state");
    setState(State::IDLE, millis());
    assignedSlot_ = -1;
    isExitOperation_ = false;
  }
}

void AccessController::emergencyStop() {
  LOG_WARN("Emergency stop triggered");
  barrier_->stop();
  setState(State::FAULT, millis());
}

const char* AccessController::getStateName() const {
  switch (state_) {
    case State::IDLE: return "IDLE";
    case State::CHECK_CAPACITY: return "CHECK_CAPACITY";
    case State::OPENING: return "OPENING";
    case State::WAIT_PASS: return "WAIT_PASS";
    case State::CLOSING: return "CLOSING";
    case State::FAULT: return "FAULT";
    default: return "UNKNOWN";
  }
}

// Private methods - State handlers

void AccessController::handleIdle(uint32_t nowMs) {
  // Verificar botones de entrada
  if (btnVip_->wasPressed()) {
    requestEntry(VehicleClass::VIP, nowMs);
    return;
  }
  
  if (btnCarga_->wasPressed()) {
    requestEntry(VehicleClass::CARGA, nowMs);
    return;
  }
  
  if (btnReg_->wasPressed()) {
    requestEntry(VehicleClass::REGULAR, nowMs);
    return;
  }
  
  // Verificar botón de salida
  if (btnExit_->wasPressed()) {
    requestExit(nowMs);
    return;
  }
}

void AccessController::handleCheckCapacity(uint32_t nowMs) {
  // Para salida, siempre permitir
  if (isExitOperation_) {
    setState(State::OPENING, nowMs);
    barrier_->open();
    return;
  }
  
  // Para entrada, verificar capacidad
  int slot = slots_->allocate(pendingClass_);
  
  if (slot >= 0) {
    // Hay espacio disponible
    assignedSlot_ = slot;
    setState(State::OPENING, nowMs);
    barrier_->open();
    
    LOG_INFO("Access granted for %s to slot %d", 
             pendingClass_ == VehicleClass::VIP ? "VIP" :
             pendingClass_ == VehicleClass::CARGA ? "CARGA" : "REGULAR",
             slot);
  } else {
    // Sin espacio disponible
    LOG_WARN("Access denied for %s - no available slots", 
             pendingClass_ == VehicleClass::VIP ? "VIP" :
             pendingClass_ == VehicleClass::CARGA ? "CARGA" : "REGULAR");
    
    setState(State::IDLE, nowMs);
  }
}

void AccessController::handleOpening(uint32_t nowMs) {
  // Verificar timeout
  if (getStateTime(nowMs) > openTimeoutMs_) {
    handleTimeout("Opening timeout", nowMs);
    return;
  }
  
  // Verificar si la barrera terminó de abrir
  if (barrier_->isOpen()) {
    setState(State::WAIT_PASS, nowMs);
    LOG_INFO("Barrier opened - waiting for vehicle to pass");
  }
}

void AccessController::handleWaitPass(uint32_t nowMs) {
  // Verificar timeout para pasar
  if (getStateTime(nowMs) > passTimeMs_) {
    LOG_INFO("Pass timeout - closing barrier");
    setState(State::CLOSING, nowMs);
    barrier_->close();
    return;
  }
  
  // TODO: En versión futura, detectar paso del vehículo con sensor adicional
  // Por ahora, esperar timeout completo
}

void AccessController::handleClosing(uint32_t nowMs) {
  // El sensor de seguridad se maneja en Barrier.update()
  
  // Verificar timeout
  if (getStateTime(nowMs) > closeTimeoutMs_) {
    handleTimeout("Closing timeout", nowMs);
    return;
  }
  
  // Verificar si la barrera terminó de cerrar
  if (barrier_->isClosed()) {
    LOG_INFO("Barrier closed - operation complete");
    setState(State::IDLE, nowMs);
    
    // Limpiar contexto
    assignedSlot_ = -1;
    isExitOperation_ = false;
  }
}

void AccessController::handleFault(uint32_t nowMs) {
  // En estado FAULT, solo esperar reset manual
  // Podríamos implementar auto-recovery después de cierto tiempo
  
  static uint32_t lastFaultLog = 0;
  if (nowMs - lastFaultLog > 10000) { // Log cada 10 segundos
    LOG_WARN("System in FAULT state - manual reset required");
    lastFaultLog = nowMs;
  }
}

// Private methods - Transitions and helpers

void AccessController::setState(State newState, uint32_t nowMs) {
  if (state_ != newState) {
    State oldState = state_;
    state_ = newState;
    stateStartMs_ = nowMs;
    
    // Log cambios de estado
    if (lastLoggedState_ != newState) {
      LOG_INFO("AccessController: %s -> %s", 
               oldState == State::IDLE ? "IDLE" :
               oldState == State::CHECK_CAPACITY ? "CHECK_CAPACITY" :
               oldState == State::OPENING ? "OPENING" :
               oldState == State::WAIT_PASS ? "WAIT_PASS" :
               oldState == State::CLOSING ? "CLOSING" : "FAULT",
               getStateName());
      lastLoggedState_ = newState;
    }
  }
}

void AccessController::requestEntry(VehicleClass vc, uint32_t nowMs) {
  if (state_ != State::IDLE) {
    LOG_WARN("Entry request ignored - system busy");
    return;
  }
  
  pendingClass_ = vc;
  isExitOperation_ = false;
  assignedSlot_ = -1;
  
  setState(State::CHECK_CAPACITY, nowMs);
  
  LOG_INFO("Entry request for %s vehicle", 
           vc == VehicleClass::VIP ? "VIP" :
           vc == VehicleClass::CARGA ? "CARGA" : "REGULAR");
}

void AccessController::requestExit(uint32_t nowMs) {
  if (state_ != State::IDLE) {
    LOG_WARN("Exit request ignored - system busy");
    return;
  }
  
  isExitOperation_ = true;
  assignedSlot_ = -1;
  
  setState(State::CHECK_CAPACITY, nowMs); // Siempre permitir salida
  LOG_INFO("Exit request received");
}

void AccessController::handleTimeout(const char* reason, uint32_t nowMs) {
  LOG_ERR("AccessController timeout: %s (state: %s, time: %lu ms)", 
          reason, getStateName(), getStateTime(nowMs));
  
  // Intentar parar la barrera y ir a FAULT
  barrier_->stop();
  setState(State::FAULT, nowMs);
}