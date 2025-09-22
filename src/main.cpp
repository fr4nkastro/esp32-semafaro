#include <Arduino.h>
#include <ESP32Servo.h>

// Core infrastructure
#include "core/Scheduler.hpp"
#include "core/Logger.hpp"
#include "core/Pins.hpp"
#include "core/Config.hpp"

// Device classes
#include "devices/Barrier.hpp"
#include "devices/ProximitySensor.hpp"
#include "devices/Button.hpp"

// Application logic
#include "app/SlotManager.hpp"
#include "app/AccessController.hpp"

// Global hardware instances
Barrier barrier;
ProximitySensor safeSensor;
Button btnVip, btnCarga, btnReg, btnExit;

// Application logic instances
SlotManager slotManager;
AccessController accessController;

// Status tracking
uint32_t lastStatusPrint = 0;
const uint32_t STATUS_INTERVAL_MS = 30000; // Print status every 30 seconds

void printSystemStatus() {
  LOG_INFO("=== SEMAFARO SYSTEM STATUS ===");
  LOG_INFO("Uptime: %lu ms", millis());
  LOG_INFO("AccessController: %s", accessController.getStateName());
  LOG_INFO("Barrier: %s", 
           barrier.isClosed() ? "CLOSED" :
           barrier.isOpen() ? "OPEN" :
           barrier.isMoving() ? "MOVING" : "FAULT");
  
  slotManager.printStatus();
  LOG_INFO("=============================");
}

void setup() {
  Serial.begin(115200);
  
  // Wait for serial to be ready
  delay(2000);
  
  LOG_INFO("=== SEMAFARO MVP Starting ===");
  LOG_INFO("ESP32-S3 Parking Control System");
  LOG_INFO("Version: 1.0.0 - MVP Implementation");
  LOG_INFO("Build: %s %s", __DATE__, __TIME__);
  
  // Initialize servo timers (ESP32-S3 specific)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  LOG_INFO("Servo timers allocated");
  
  // Initialize hardware devices
  LOG_INFO("Initializing hardware...");
  
  // Barrier with safety sensor
  barrier.begin(Pins::SERVO_PWM);
  barrier.setAngles(Cfg::kServoClosedDeg, Cfg::kServoOpenDeg);
  
  // Safety sensor
  safeSensor.begin(Pins::BARRIER_SAFE_IN, true, true); // PNP with pullup
  
  // Button inputs
  btnVip.begin(Pins::BTN_VIP_IN, true, true);     // Pullup, active-low
  btnCarga.begin(Pins::BTN_CARGA_IN, true, true);
  btnReg.begin(Pins::BTN_REG_IN, true, true);
  btnExit.begin(Pins::BTN_EXIT, true, true);
  
  LOG_INFO("Hardware devices initialized");
  
  // Initialize application logic
  LOG_INFO("Initializing application logic...");
  
  slotManager.begin();
  accessController.begin(&barrier, &slotManager, 
                        &btnVip, &btnCarga, &btnReg, &btnExit, 
                        &safeSensor);
  
  LOG_INFO("Application logic initialized");
  
  // Setup scheduler tasks
  LOG_INFO("Setting up scheduler tasks...");
  
  // Main control loop - 20Hz (50ms)
  Scheduler::every(Cfg::kMainUpdateMs, []() {
    uint32_t now = millis();
    
    // Update all devices and logic
    slotManager.update(now);
    accessController.update(now);
    barrier.update(now, safeSensor.isDetected(now));
  });
  
  // Status monitoring - every 30 seconds
  Scheduler::every(STATUS_INTERVAL_MS, []() {
    printSystemStatus();
  });
  
  // Watchdog-style alive signal - every 5 seconds
  Scheduler::every(5000, []() {
    LOG_DEBUG("System alive - free heap: %d bytes", ESP.getFreeHeap());
  });
  
  LOG_INFO("Scheduler configured");
  
  // Print initial system status
  printSystemStatus();
  
  LOG_INFO("=== SEMAFARO MVP Ready ===");
  LOG_INFO("System is operational and waiting for input");
  LOG_INFO("Press VIP/CARGA/REGULAR buttons to request entry");
  LOG_INFO("Press EXIT button to request exit");
  LOG_INFO("=============================");
}

void loop() {
  // Execute all scheduled tasks
  Scheduler::tick();
  
  // Small delay to prevent watchdog timeout
  delay(1);
}