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

## Critical Development Patterns

### Pin Assignment Strategy
All pin mappings are centralized in `core/Pins.hpp` using `constexpr` constants:
```cpp
namespace Pins {
  constexpr uint8_t SERVO_PWM = 5;
  constexpr uint8_t BTN_VIP_IN = 6;
  constexpr uint8_t BARRIER_SAFE_IN = 10;
  constexpr uint8_t S_VIP1 = 11;  // Slot sensors
  
  struct TL { uint8_t RED; uint8_t GREEN; };
  constexpr TL TL_VIP1{17, 18};
  constexpr TL TL_CARG1{21, 35};  // Note ESP32-S3 PWM pins
}
```

### Device Class Pattern
Every hardware device follows this structure:
- `begin(pins...)` - initialize hardware with pin ownership
- `update(uint32_t nowMs, ...)` - non-blocking state updates
- Public methods for commands (open/close, setOccupied/setFree)
- Private state management with debouncing for inputs
- State enums with `getState()` methods for debugging

### Critical ESP32-S3 Setup Pattern
**Always allocate servo timers in setup() before any servo initialization:**
```cpp
// Required in main.cpp setup() - ESP32-S3 specific
ESP32PWM::allocateTimer(0);
ESP32PWM::allocateTimer(1);
ESP32PWM::allocateTimer(2);
ESP32PWM::allocateTimer(3);
```

### Scheduler Usage
Main loop uses cooperative multitasking with specific update pattern:
```cpp
// Setup multiple scheduled tasks in setup()
Scheduler::every(Cfg::kMainUpdateMs, []() {  // 50ms = 20Hz
  uint32_t now = millis();
  slotManager.update(now);
  accessController.update(now);
  barrier.update(now, safeSensor.isDetected(now));
});

// Status monitoring task
Scheduler::every(30000, []() { printSystemStatus(); });

// In loop() - just call tick
void loop() {
  Scheduler::tick();
  delay(1);  // Prevent watchdog timeout
}
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
- **VIP Priority**: Can use CARGA or REGULAR slots when VIP full (configurable fallback policy)
- **CARGA/REGULAR Restriction**: Cannot use VIP slots, blocked when their type is full
- **Implementation**: `SlotManager::allocate(VehicleClass vc)` with `findVipFallback()` method
- **Policy Configuration**: `Cfg::kVipFallback` enum in `core/Config.hpp`

### State Machine Flow
AccessController FSM: `IDLE → CHECK_CAPACITY → OPENING → WAIT_PASS → CLOSING → IDLE`
- **Fault Recovery**: `FAULT` state with `reset()` method for manual recovery
- **Safety Integration**: `safeSensorActive` parameter prevents closing
- **Debugging Support**: `getStateName()`, `getStateTime()`, state logging
- **Timeout Handling**: Separate timeouts for open/close operations in `core/Config.hpp`

## Development Workflow

### Build & Deploy
```bash
# Compile and upload (PowerShell on Windows)
pio run -e 4d_systems_esp32s3_gen4_r8n16 -t upload

# Monitor serial output (115200 baud)
pio device monitor -e 4d_systems_esp32s3_gen4_r8n16

# Clean build if needed
pio run -e 4d_systems_esp32s3_gen4_r8n16 -t clean
```

### Testing Strategy
- Unit tests in `test/` for SlotManager allocation logic
- Hardware validation using example implementations in `docs/ejemplo-servo.md`
- Use PlatformIO Unity framework for logic testing

## ESP32-S3 Specific Considerations

### PWM Pin Compatibility
ESP32-S3 PWM-capable pins: 1-21, 35-45, 47-48 (avoid 0=button, 48=LED)
**Critical**: Pins 35+ used for traffic lights in current design

### Power Management
- **Servo**: Requires external 5V supply (1000mA peak)
- **Proximity Sensors**: 10-30V PNP sensors need 3.3V level conversion
- **LEDs**: Use 220Ω resistors for current limiting (~10mA)

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
1. **Never block execution** - use update() patterns, not delay() (except 1ms watchdog delay)
2. **Single pin ownership** - only device classes should write to their pins
3. **Debounce all inputs** - buttons and sensors need 30ms filtering
4. **Timeout all FSM states** - include fault recovery with manual reset
5. **Test allocation logic** - VIP fallback and capacity rules are complex
6. **Servo timer allocation** - must call `ESP32PWM::allocateTimer()` before servo init
7. **Safety sensor integration** - always pass to `barrier.update(now, safeSensorActive)`

## Debugging Features
- **Status printing**: `printSystemStatus()` every 30 seconds with full state
- **State logging**: Each FSM transition logs with `getStateName()`
- **Log levels**: Use `LOG_INFO/WARN/ERROR/DEBUG` with `LOG_LEVEL` build flag
- **Heap monitoring**: System prints free heap every 5 seconds