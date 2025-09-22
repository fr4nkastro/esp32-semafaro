#pragma once
#include <array>
#include "core/Types.hpp"
#include "devices/TrafficLight.hpp"
#include "devices/ProximitySensor.hpp"
#include "core/Config.hpp"

struct Slot {
  SlotType type;
  SlotState state;
  ProximitySensor sensor;
  TrafficLight trafficLight;
  
  // Para debugging
  uint8_t id;
  const char* name;
};

class SlotManager {
public:
  static constexpr size_t kSlots = 6;
  static constexpr size_t kSlotsPerType = 2;

  void begin();
  void update(uint32_t nowMs);

  // Asignación según reglas FASE 1/2
  int allocate(VehicleClass vc);   // Retorna índice de slot o -1 si no hay
  void releaseByIndex(int idx);    // Para liberar en salida manual
  void releaseAll();               // Para resetear sistema

  // Consulta de estado
  bool allFull() const;
  size_t freeCount(SlotType t) const;
  size_t occupiedCount(SlotType t) const;
  size_t totalFreeCount() const;
  size_t totalOccupiedCount() const;

  // Debug y monitoreo  
  SlotState getSlotState(int idx) const;
  SlotType getSlotType(int idx) const;
  const char* getSlotName(int idx) const;
  void printStatus() const;

private:
  // Algoritmos de búsqueda
  int findSameClass(VehicleClass vc) const;
  int findVipFallback() const; // VIP → CARGA o REGULAR según política

  // Helpers
  SlotType vehicleClassToSlotType(VehicleClass vc) const;
  void updateSlotState(int idx, uint32_t nowMs);
  void onSlotOccupied(int idx);
  void onSlotFreed(int idx);

  std::array<Slot, kSlots> slots_;
  bool initialized_{false};
};