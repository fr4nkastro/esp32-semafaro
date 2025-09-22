#pragma once
#include <stdint.h>

// Tipos comunes para el proyecto SEMAFARO
enum class VehicleClass : uint8_t { 
  VIP, 
  CARGA, 
  REGULAR 
};

enum class SlotType : uint8_t { 
  VIP, 
  CARGA, 
  REGULAR 
};

enum class SlotState : uint8_t { 
  FREE, 
  OCCUPIED 
};

// Estados de la barrera
enum class BarrierState : uint8_t {
  CLOSED,
  OPENING, 
  OPEN,
  CLOSING,
  FAULT
};

// Eventos del sistema
enum class SystemEvent : uint8_t {
  BTN_VIP_PRESSED,
  BTN_CARGA_PRESSED,
  BTN_REGULAR_PRESSED,
  BTN_EXIT_PRESSED,
  BARRIER_OPENED,
  BARRIER_CLOSED,
  BARRIER_TIMEOUT,
  SLOT_OCCUPIED,
  SLOT_FREED,
  SAFETY_SENSOR_ACTIVE,
  SYSTEM_FAULT
};