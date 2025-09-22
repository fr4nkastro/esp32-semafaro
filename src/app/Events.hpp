#pragma once
#include "core/Types.hpp"

// Eventos del sistema SEMAFARO
namespace Events {
  
  // Eventos de entrada
  struct ButtonPressed {
    VehicleClass vehicleClass;
    uint32_t timestamp;
  };
  
  struct ExitButtonPressed {
    uint32_t timestamp;
  };
  
  // Eventos de barrera
  struct BarrierOpened {
    uint32_t timestamp;
  };
  
  struct BarrierClosed {
    uint32_t timestamp;
  };
  
  struct BarrierTimeout {
    BarrierState state;
    uint32_t timestamp;
  };
  
  // Eventos de slots
  struct SlotOccupied {
    int slotIndex;
    SlotType slotType;
    uint32_t timestamp;
  };
  
  struct SlotFreed {
    int slotIndex;
    SlotType slotType;
    uint32_t timestamp;
  };
  
  // Eventos de seguridad
  struct SafetySensorActivated {
    uint32_t timestamp;
  };
  
  struct SafetySensorDeactivated {
    uint32_t timestamp;
  };
  
  // Eventos de sistema
  struct SystemFault {
    const char* description;
    uint32_t timestamp;
  };
  
  struct CapacityFull {
    VehicleClass deniedClass;
    uint32_t timestamp;
  };
}