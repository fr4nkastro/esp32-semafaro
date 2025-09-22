#pragma once
#include <Arduino.h>

namespace Pins {
  // Servo barrera
  constexpr uint8_t SERVO_PWM = 5;      // LEDC channel assigned in Barrier.cpp

  // Botoneras
  constexpr uint8_t BTN_VIP_IN   = 6;
  constexpr uint8_t BTN_CARGA_IN = 7;
  constexpr uint8_t BTN_REG_IN   = 8;
  constexpr uint8_t BTN_EXIT     = 9;

  // Sensor seguridad barrera
  constexpr uint8_t BARRIER_SAFE_IN = 10;

  // Sensores de slots
  constexpr uint8_t S_VIP1  = 11;
  constexpr uint8_t S_VIP2  = 12;
  constexpr uint8_t S_CARG1 = 13;
  constexpr uint8_t S_CARG2 = 14;
  constexpr uint8_t S_REG1  = 15;
  constexpr uint8_t S_REG2  = 16;

  // Semáforos (estructura para rojo/verde por slot)
  struct TL { uint8_t RED; uint8_t GREEN; };
  constexpr TL TL_VIP1  {17, 18};
  constexpr TL TL_VIP2  {19, 20};
  constexpr TL TL_CARG1 {21, 35};  // Usando pines PWM compatibles ESP32-S3
  constexpr TL TL_CARG2 {36, 37};
  constexpr TL TL_REG1  {38, 39};
  constexpr TL TL_REG2  {40, 41};
}