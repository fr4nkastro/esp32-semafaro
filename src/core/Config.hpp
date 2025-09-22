#pragma once
#include <stdint.h>

namespace Cfg {
  // Tiempos ajustables
  constexpr uint32_t kPassTimeMs   = 3000;  // Tiempo para pasar después de abrir
  constexpr uint32_t kOpenTimeout  = 5000;  // Timeout para apertura de barrera
  constexpr uint32_t kCloseTimeout = 3000;  // Timeout para cierre de barrera
  constexpr uint32_t kBarrierStepMs = 20;   // Tiempo entre pasos del servo (movimiento suave)

  // Debounces
  constexpr uint16_t kBtnDebounceMs = 30;   // Debounce para botones
  constexpr uint16_t kSensDebounceMs = 30;  // Debounce para sensores inductivos

  // Configuración servo
  constexpr uint8_t kServoClosedDeg = 10;   // Ángulo barrera cerrada
  constexpr uint8_t kServoOpenDeg = 90;     // Ángulo barrera abierta
  constexpr uint8_t kServoStepDeg = 2;      // Paso de movimiento por tick

  // Configuración del scheduler
  constexpr uint32_t kMainUpdateMs = 50;    // 20Hz para update principal

  // Política FASE 2: VIP fallback primero a CARGA, luego REGULAR
  enum class VipFallbackPolicy { CARGA_THEN_REGULAR, REGULAR_THEN_CARGA };
  constexpr VipFallbackPolicy kVipFallback = VipFallbackPolicy::CARGA_THEN_REGULAR;
}