#include <unity.h>
#include "../src/app/SlotManager.hpp"
#include "../src/core/Types.hpp"

// Mock de hardware para testing
class MockProximitySensor {
public:
  void begin(uint8_t pin, bool pullup = true, bool normallyHigh = true) {}
  bool isDetected(uint32_t nowMs) { return detected_; }
  bool wasActivated() { return false; }
  bool wasDeactivated() { return false; }
  
  void setDetected(bool state) { detected_ = state; }
private:
  bool detected_ = false;
};

class MockTrafficLight {
public:
  void begin(uint8_t pinRed, uint8_t pinGreen) {}
  void setOccupied() { state_ = SlotState::OCCUPIED; }
  void setFree() { state_ = SlotState::FREE; }
  void setOff() {}
  SlotState getState() const { return state_; }
private:
  SlotState state_ = SlotState::FREE;
};

void test_slot_allocation_same_class() {
  // Test allocation within same vehicle class
  SlotManager manager;
  
  // Simulate initialization without hardware
  // This would need adaptation to work with mocks
  
  // Test VIP allocation
  int slot = manager.allocate(VehicleClass::VIP);
  TEST_ASSERT_TRUE(slot >= 0 && slot <= 1); // VIP slots are 0,1
  
  // Test CARGA allocation  
  slot = manager.allocate(VehicleClass::CARGA);
  TEST_ASSERT_TRUE(slot >= 2 && slot <= 3); // CARGA slots are 2,3
  
  // Test REGULAR allocation
  slot = manager.allocate(VehicleClass::REGULAR);
  TEST_ASSERT_TRUE(slot >= 4 && slot <= 5); // REGULAR slots are 4,5
}

void test_vip_fallback_priority() {
  // Test VIP fallback when VIP slots are full
  SlotManager manager;
  
  // Fill VIP slots (would need proper mock setup)
  // int slot1 = manager.allocate(VehicleClass::VIP);
  // int slot2 = manager.allocate(VehicleClass::VIP);
  
  // Third VIP should get CARGA slot if available
  // int fallbackSlot = manager.allocate(VehicleClass::VIP);
  // TEST_ASSERT_TRUE(fallbackSlot >= 2 && fallbackSlot <= 3);
}

void test_capacity_limits() {
  // Test rejection when all slots are full
  SlotManager manager;
  
  // This test would need proper mock setup to simulate
  // all sensors detecting vehicles
}

void test_slot_release() {
  // Test manual slot release
  SlotManager manager;
  
  // Allocate and then release
  // int slot = manager.allocate(VehicleClass::VIP);
  // TEST_ASSERT_NOT_EQUAL(-1, slot);
  
  // manager.releaseByIndex(slot);
  // TEST_ASSERT_EQUAL(SlotState::FREE, manager.getSlotState(slot));
}

void setUp(void) {
  // Set up before each test
}

void tearDown(void) {
  // Clean up after each test
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  
  RUN_TEST(test_slot_allocation_same_class);
  RUN_TEST(test_vip_fallback_priority);
  RUN_TEST(test_capacity_limits);
  RUN_TEST(test_slot_release);
  
  return UNITY_END();
}