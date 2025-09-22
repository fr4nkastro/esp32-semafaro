# SEMAFARO - ESP32-S3 Parking Control System

## Project Overview
This is an ESP32-S3 based automated parking control system with barrier, proximity sensors, traffic lights, and buttons. The project uses a **hybrid architecture** combining OOP device encapsulation with structured finite state machines for control logic.

## Key Architecture Principles

### Hybrid Design Pattern
- **OOP for Hardware**: Each device (servo, sensors, buttons, traffic lights) is encapsulated in classes with exclusive pin ownership
- **FSM for Control Logic**: Complex flows (access control, slot management) use explicit state machines
- **Non-blocking Execution**: Everything runs cooperatively using `Scheduler::tick()` and `update(nowMs)` patterns - **never use `delay()`**

### Hardware Specifications
- **Board**: ESP32-S3 (4d_systems_esp32s3_gen4_r8n16)
- **Servo**: SG90 (barrier control) via ESP32Servo library
- **Sensors**: 7x inductive proximity sensors (6 parking slots + 1 safety)
- **Traffic Lights**: 6x dual LED modules (red/green per slot)
- **Buttons**: 4x (VIP, CARGA, REGULAR entry + EXIT)
- **Relays**: 8-channel module for traffic light control

## Critical Development Patterns

### Pin Assignment Strategy
All pin mappings are centralized in `core/Pins.hpp` using `constexpr` constants:
```cpp
namespace Pins {
  constexpr uint8_t SERVO_PWM = 5;
  constexpr uint8_t BTN_VIP_IN = 6;
  struct TL { uint8_t RED; uint8_t GREEN; };
  constexpr TL TL_VIP1{17, 18};
}
```

### Device Class Pattern
Every hardware device follows this structure:
- `begin(pins...)` - initialize hardware
- `update(uint32_t nowMs, ...)` - non-blocking state updates
- Public methods for commands (open/close, setOccupied/setFree)
- Private state management with debouncing for inputs

### Scheduler Usage
Main loop uses cooperative multitasking:
```cpp
Scheduler::every(50, [](){  // 20Hz update rate
  uint32_t now = millis();
  slots.update(now);
  controller.update(now);
  barrier.update(now, safe.read(now));
});
```

## Project Structure Conventions

### Required Folder Structure
```
src/
├── main.cpp                    // Entry point with Scheduler setup
├── devices/                    // Hardware abstraction classes
│   ├── Barrier.hpp/.cpp       // Servo + safety sensor integration
│   ├── TrafficLight.hpp/.cpp  // Dual LED control per slot
│   ├── ProximitySensor.hpp/.cpp// Inductive sensors with debounce
│   └── Button.hpp/.cpp        // Button input with debounce
├── app/                        // Business logic layer
│   ├── AccessController.hpp/.cpp  // FSM for barrier operations
│   ├── SlotManager.hpp/.cpp       // 6-slot allocation logic
│   └── Events.hpp                 // Event definitions
└── core/                       // Infrastructure layer
    ├── Scheduler.hpp/.cpp         // Non-blocking task scheduler
    ├── Pins.hpp                   // Centralized pin definitions
    ├── Config.hpp                 // Timing constants and parameters
    └── Logger.hpp                 // Debug logging macros
```

## Business Logic Rules

### Slot Allocation (Phase 1)
- 6 slots total: 2 VIP, 2 CARGA, 2 REGULAR
- Button press → check availability → open barrier if space exists
- Each slot has inductive sensor + red/green traffic light
- Entry blocked when all 6 slots occupied

### Priority System (Phase 2)
- **VIP Priority**: Can use CARGA or REGULAR slots when VIP full
- **CARGA/REGULAR Restriction**: Cannot use VIP slots, blocked when their type is full
- Implementation in `SlotManager::allocate(VehicleClass vc)`

### State Machine Flow
AccessController FSM: `IDLE → CHECK_CAPACITY → OPENING → WAIT_PASS → CLOSING → IDLE`
- Includes timeout handling and safety sensor integration
- Fault state for error recovery

## Development Workflow

### Build & Deploy
```bash
# Compile and upload
pio run -e 4d_systems_esp32s3_gen4_r8n16 -t upload

# Monitor serial output
pio device monitor -e 4d_systems_esp32s3_gen4_r8n16
```

### Testing Strategy
- Unit tests in `test/` for SlotManager allocation logic
- Hardware validation using example implementations in `docs/ejemplo-servo.md`
- Use PlatformIO Unity framework for logic testing

## ESP32-S3 Specific Considerations

### PWM Pin Compatibility
ESP32-S3 PWM-capable pins: 1-21, 35-45, 47-48 (avoid 0=button, 48=LED)
Always allocate timers in setup():
```cpp
ESP32PWM::allocateTimer(0);
ESP32PWM::allocateTimer(1);
ESP32PWM::allocateTimer(2);
ESP32PWM::allocateTimer(3);
```

### Power Management
- Servo requires external 5V supply (not ESP32 3.3V)
- Inductive sensors may need level conversion for 3.3V compatibility
- Relay module isolation recommended for traffic lights

## Critical Files to Reference
- `docs/PRD-MVP.md` - Complete requirements and architecture
- `docs/lineamientos.md` - Detailed coding guidelines and patterns
- `docs/ejemplo-servo.md` - Hardware implementation examples
- `platformio.ini` - Board configuration and dependencies

## Common Pitfalls to Avoid
1. **Never block execution** - use update() patterns, not delay()
2. **Single pin ownership** - only device classes should write to their pins
3. **Debounce all inputs** - buttons and sensors need filtering
4. **Timeout all FSM states** - include fault recovery paths
5. **Test allocation logic** - slot assignment has complex business rules