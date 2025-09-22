#include "SlotManager.hpp"
#include "core/Logger.hpp"
#include "core/Pins.hpp"

void SlotManager::begin() {
  // Configurar los 6 slots según el layout:
  // 0,1: VIP | 2,3: CARGA | 4,5: REGULAR
  
  slots_[0] = {SlotType::VIP, SlotState::FREE, {}, {}, 0, "VIP1"};
  slots_[1] = {SlotType::VIP, SlotState::FREE, {}, {}, 1, "VIP2"};
  slots_[2] = {SlotType::CARGA, SlotState::FREE, {}, {}, 2, "CARGA1"};
  slots_[3] = {SlotType::CARGA, SlotState::FREE, {}, {}, 3, "CARGA2"};
  slots_[4] = {SlotType::REGULAR, SlotState::FREE, {}, {}, 4, "REG1"};
  slots_[5] = {SlotType::REGULAR, SlotState::FREE, {}, {}, 5, "REG2"};

  // Configurar sensores y semáforos
  slots_[0].sensor.begin(Pins::S_VIP1, true, true);      // PNP, pullup
  slots_[0].trafficLight.begin(Pins::TL_VIP1.RED, Pins::TL_VIP1.GREEN);
  
  slots_[1].sensor.begin(Pins::S_VIP2, true, true);
  slots_[1].trafficLight.begin(Pins::TL_VIP2.RED, Pins::TL_VIP2.GREEN);
  
  slots_[2].sensor.begin(Pins::S_CARG1, true, true);
  slots_[2].trafficLight.begin(Pins::TL_CARG1.RED, Pins::TL_CARG1.GREEN);
  
  slots_[3].sensor.begin(Pins::S_CARG2, true, true);
  slots_[3].trafficLight.begin(Pins::TL_CARG2.RED, Pins::TL_CARG2.GREEN);
  
  slots_[4].sensor.begin(Pins::S_REG1, true, true);
  slots_[4].trafficLight.begin(Pins::TL_REG1.RED, Pins::TL_REG1.GREEN);
  
  slots_[5].sensor.begin(Pins::S_REG2, true, true);
  slots_[5].trafficLight.begin(Pins::TL_REG2.RED, Pins::TL_REG2.GREEN);

  initialized_ = true;
  
  LOG_INFO("SlotManager initialized with %d slots", kSlots);
  printStatus();
}

void SlotManager::update(uint32_t nowMs) {
  if (!initialized_) return;

  // Actualizar estado de todos los slots basado en sensores
  for (int i = 0; i < kSlots; i++) {
    updateSlotState(i, nowMs);
  }
}

int SlotManager::allocate(VehicleClass vc) {
  if (!initialized_) {
    LOG_ERR("SlotManager not initialized");
    return -1;
  }

  // Estrategia FASE 1 + FASE 2:
  // 1. Intentar slot de su misma clase
  int slot = findSameClass(vc);
  if (slot >= 0) {
    LOG_INFO("Allocated slot %d (%s) for %s vehicle", 
             slot, slots_[slot].name, 
             vc == VehicleClass::VIP ? "VIP" : 
             vc == VehicleClass::CARGA ? "CARGA" : "REGULAR");
    return slot;
  }

  // 2. Si es VIP y no hay espacios VIP, buscar fallback
  if (vc == VehicleClass::VIP) {
    slot = findVipFallback();
    if (slot >= 0) {
      LOG_INFO("VIP fallback: allocated slot %d (%s) for VIP vehicle", 
               slot, slots_[slot].name);
      return slot;
    }
  }

  // 3. No hay espacios disponibles
  LOG_WARN("No available slots for %s vehicle", 
           vc == VehicleClass::VIP ? "VIP" : 
           vc == VehicleClass::CARGA ? "CARGA" : "REGULAR");
  return -1;
}

void SlotManager::releaseByIndex(int idx) {
  if (idx < 0 || idx >= kSlots) {
    LOG_ERR("Invalid slot index: %d", idx);
    return;
  }

  if (slots_[idx].state == SlotState::OCCUPIED) {
    slots_[idx].state = SlotState::FREE;
    slots_[idx].trafficLight.setFree();
    LOG_INFO("Manually released slot %d (%s)", idx, slots_[idx].name);
  }
}

void SlotManager::releaseAll() {
  for (int i = 0; i < kSlots; i++) {
    releaseByIndex(i);
  }
  LOG_INFO("All slots manually released");
}

bool SlotManager::allFull() const {
  return totalFreeCount() == 0;
}

size_t SlotManager::freeCount(SlotType t) const {
  size_t count = 0;
  for (const auto& slot : slots_) {
    if (slot.type == t && slot.state == SlotState::FREE) {
      count++;
    }
  }
  return count;
}

size_t SlotManager::occupiedCount(SlotType t) const {
  return kSlotsPerType - freeCount(t);
}

size_t SlotManager::totalFreeCount() const {
  size_t count = 0;
  for (const auto& slot : slots_) {
    if (slot.state == SlotState::FREE) {
      count++;
    }
  }
  return count;
}

size_t SlotManager::totalOccupiedCount() const {
  return kSlots - totalFreeCount();
}

SlotState SlotManager::getSlotState(int idx) const {
  if (idx < 0 || idx >= kSlots) return SlotState::FREE;
  return slots_[idx].state;
}

SlotType SlotManager::getSlotType(int idx) const {
  if (idx < 0 || idx >= kSlots) return SlotType::REGULAR;
  return slots_[idx].type;
}

const char* SlotManager::getSlotName(int idx) const {
  if (idx < 0 || idx >= kSlots) return "INVALID";
  return slots_[idx].name;
}

void SlotManager::printStatus() const {
  LOG_INFO("=== SLOT STATUS ===");
  for (int i = 0; i < kSlots; i++) {
    const auto& slot = slots_[i];
    LOG_INFO("Slot %d (%s): %s %s", 
             i, slot.name,
             slot.type == SlotType::VIP ? "VIP" :
             slot.type == SlotType::CARGA ? "CARGA" : "REG",
             slot.state == SlotState::FREE ? "FREE" : "OCCUPIED");
  }
  LOG_INFO("Total: %d/%d occupied", totalOccupiedCount(), kSlots);
}

// Private methods

int SlotManager::findSameClass(VehicleClass vc) const {
  SlotType targetType = vehicleClassToSlotType(vc);
  
  for (int i = 0; i < kSlots; i++) {
    if (slots_[i].type == targetType && slots_[i].state == SlotState::FREE) {
      return i;
    }
  }
  
  return -1; // No encontrado
}

int SlotManager::findVipFallback() const {
  // Política: VIP puede usar CARGA primero, luego REGULAR
  if (Cfg::kVipFallback == Cfg::VipFallbackPolicy::CARGA_THEN_REGULAR) {
    // Buscar CARGA libre
    int slot = findSameClass(VehicleClass::CARGA);
    if (slot >= 0) return slot;
    
    // Buscar REGULAR libre
    return findSameClass(VehicleClass::REGULAR);
  } else {
    // REGULAR_THEN_CARGA
    int slot = findSameClass(VehicleClass::REGULAR);
    if (slot >= 0) return slot;
    
    return findSameClass(VehicleClass::CARGA);
  }
}

SlotType SlotManager::vehicleClassToSlotType(VehicleClass vc) const {
  switch (vc) {
    case VehicleClass::VIP: return SlotType::VIP;
    case VehicleClass::CARGA: return SlotType::CARGA;
    case VehicleClass::REGULAR: return SlotType::REGULAR;
    default: return SlotType::REGULAR;
  }
}

void SlotManager::updateSlotState(int idx, uint32_t nowMs) {
  auto& slot = slots_[idx];
  bool detected = slot.sensor.isDetected(nowMs);
  
  // Detectar cambios de estado
  if (detected && slot.state == SlotState::FREE) {
    onSlotOccupied(idx);
  } else if (!detected && slot.state == SlotState::OCCUPIED) {
    onSlotFreed(idx);
  }
}

void SlotManager::onSlotOccupied(int idx) {
  auto& slot = slots_[idx];
  slot.state = SlotState::OCCUPIED;
  slot.trafficLight.setOccupied();
  
  LOG_INFO("Slot %d (%s) OCCUPIED", idx, slot.name);
}

void SlotManager::onSlotFreed(int idx) {
  auto& slot = slots_[idx];
  slot.state = SlotState::FREE;
  slot.trafficLight.setFree();
  
  LOG_INFO("Slot %d (%s) FREED", idx, slot.name);
}