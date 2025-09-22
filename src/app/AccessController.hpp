#pragma once
#include "core/Types.hpp"
#include "devices/Barrier.hpp"
#include "app/SlotManager.hpp"
#include "devices/Button.hpp"
#include "devices/ProximitySensor.hpp"
#include "core/Config.hpp"

class AccessController {
public:
  enum class State : uint8_t { 
    IDLE, 
    CHECK_CAPACITY, 
    OPENING, 
    WAIT_PASS, 
    CLOSING, 
    FAULT 
  };

  void begin(Barrier* barrier, SlotManager* slots,
             Button* btnVip, Button* btnCarga, Button* btnReg, Button* btnExit,
             ProximitySensor* safe);
  
  void update(uint32_t nowMs);
  
  // Estado actual
  State getState() const { return state_; }
  bool isIdle() const { return state_ == State::IDLE; }
  bool isFault() const { return state_ == State::FAULT; }
  
  // Control manual
  void reset(); // Salir de FAULT y volver a IDLE
  void emergencyStop(); // Parar barrera inmediatamente
  
  // Para debugging
  const char* getStateName() const;
  VehicleClass getPendingClass() const { return pendingClass_; }
  int getAssignedSlot() const { return assignedSlot_; }
  uint32_t getStateTime(uint32_t nowMs) const { return nowMs - stateStartMs_; }

private:
  // Handlers de estado
  void handleIdle(uint32_t nowMs);
  void handleCheckCapacity(uint32_t nowMs);
  void handleOpening(uint32_t nowMs);
  void handleWaitPass(uint32_t nowMs);
  void handleClosing(uint32_t nowMs);
  void handleFault(uint32_t nowMs);
  
  // Transiciones
  void setState(State newState, uint32_t nowMs);
  void requestEntry(VehicleClass vc, uint32_t nowMs);
  void requestExit(uint32_t nowMs);
  void handleTimeout(const char* reason, uint32_t nowMs);

  // Referencias a hardware y lógica
  Barrier* barrier_{nullptr};
  SlotManager* slots_{nullptr};
  Button* btnVip_{nullptr};
  Button* btnCarga_{nullptr};
  Button* btnReg_{nullptr};
  Button* btnExit_{nullptr};
  ProximitySensor* safe_{nullptr};
  
  // Estado de la FSM
  State state_{State::IDLE};
  State lastLoggedState_{State::IDLE};
  uint32_t stateStartMs_{0};
  
  // Contexto de la operación actual
  VehicleClass pendingClass_{VehicleClass::REGULAR};
  int assignedSlot_{-1};
  bool isExitOperation_{false};
  
  // Timeouts
  const uint32_t passTimeMs_ = Cfg::kPassTimeMs;
  const uint32_t openTimeoutMs_ = Cfg::kOpenTimeout;
  const uint32_t closeTimeoutMs_ = Cfg::kCloseTimeout;
  
  // Para evitar spam de logs
  bool safeSensorLastState_{false};
  bool initialized_{false};
};