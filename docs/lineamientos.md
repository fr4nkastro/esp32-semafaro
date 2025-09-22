¡Perfecto! Te propongo una **plantilla híbrida** (estructurada + POO) para ESP32-S3 con PlatformIO/Arduino, pensada para crecer ordenadamente, probar en etapas y mantener el código limpio. Incluye arquitectura, lineamientos, file-tree, configuración base y esqueletos de clases/FSM para tu proyecto de parqueo (FASE 1 y FASE 2).

---

# 1) Enfoque de arquitectura

**Híbrido recomendado:**

* **POO** para encapsular dispositivos/actuadores/sensores (dueño único de pines y lógica de bajo nivel).
* **FSMs estructuradas** (con `enum class` + `switch` no bloqueante) para orquestar **flujos de operación** (Acceso/Asignación de espacios).
* **Capa de coordinación** (Controlador) que consulta **estado global** (ocupación, prioridad) y envía **comandos** a los dispositivos.
* **Scheduler cooperativo** (basado en `millis()`) para tareas periódicas sin `delay()`.

Beneficio: cada actuador tiene una API estable (POO) y las secuencias siguen un **CASE** legible (estructurado). La separación evita que varias partes escriban el mismo pin.

---

# 2) Estructura de carpetas (PlatformIO)

```
parking-fsm-esp32s3/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ app/
│  │  ├─ AccessController.hpp / .cpp        // FSM de la barrera/entrada/salida
│  │  ├─ SlotManager.hpp / .cpp             // Gestión de 6 slots y reglas FASE 2
│  │  └─ Events.hpp                         // Definición de eventos (enum)
│  ├─ core/
│  │  ├─ Pins.hpp                           // Mapa de pines centralizado
│  │  ├─ Scheduler.hpp / .cpp               // Tareas no bloqueantes (tick)
│  │  ├─ Logger.hpp                         // Niveles de log (INFO/WARN/ERR)
│  │  ├─ Config.hpp                         // Constantes/parametros (tiempos, PWMs)
│  │  └─ Types.hpp                          // Tipos comunes (enums, structs)
│  └─ devices/
│     ├─ Barrier.hpp / .cpp                 // Servo + sensor antiaplastamiento
│     ├─ TrafficLight.hpp / .cpp            // Rojo/Verde por slot
│     ├─ ProximitySensor.hpp / .cpp         // Inductivos (con debounce)
│     ├─ Button.hpp / .cpp                  // Botoneras (debounce)
│     └─ Relay.hpp / .cpp                   // Módulo de relés (si fuese necesario)
├─ include/                                 // headers compartidos (opcional)
├─ test/
│  └─ test_slot_manager/                    // pruebas unitarias (Unity)
└─ docs/
   ├─ README.md                             // Cómo correr, cómo extender
   ├─ Design.md                             // Diagramas y FSMs
   └─ IO-Map.md                             // Mapa de IO y cableado
```

---

# 3) `platformio.ini` (ESP32-S3 + Arduino + servo)

```ini
[env:esp32s3]
platform = espressif32
board = esp32s3box          ; ajusta al modelo exacto de tu S3
framework = arduino
monitor_speed = 115200
build_flags =
  -DCORE_DEBUG_LEVEL=3
  -DLOG_LEVEL_INFO          ; control de nivel de log
lib_deps =
  madhephaestus/ESP32Servo  ; Servo en ESP32 (LEDC)
  ; opcional: thomasfredericks/Bounce2  ; si prefieres librería de debounce
```

> Si tu placa no es `esp32s3box`, cambia `board` por el de tu módulo.

---

# 4) Mapa de IO (ejemplo sugerido)

> Ajusta a tu hardware real y documenta en `docs/IO-Map.md`.

* **Barrera (Servo SG90)**: `SERVO_PWM` (LEDC)
* **Sensor antiaplastamiento**: `BARRIER_SAFE_IN` (digital IN, con pullup)
* **Botoneras de entrada**: `BTN_VIP_IN`, `BTN_CARGA_IN`, `BTN_REG_IN`
* **Botonera de salida**: `BTN_EXIT`
* **Sensores de slot** (6 inductivos): `S_VIP1`, `S_VIP2`, `S_CARG1`, `S_CARG2`, `S_REG1`, `S_REG2`
* **Semáforos (6 módulos)**: dos pines por slot (`*_RED`, `*_GREEN`)
* **Módulo de relé** (si lo usas para luces o actuadores auxiliares)

Define todo en `core/Pins.hpp`:

```cpp
#pragma once
#include <Arduino.h>

namespace Pins {
  // Servo barrera
  constexpr uint8_t SERVO_PWM = 5;      // LEDC channel asigned in Barrier.cpp

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

  // Semáforos (ejemplo)
  struct TL { uint8_t RED; uint8_t GREEN; };
  constexpr TL TL_VIP1  {17, 18};
  constexpr TL TL_VIP2  {19, 20};
  constexpr TL TL_CARG1 {21, 22};
  constexpr TL TL_CARG2 {23, 24};
  constexpr TL TL_REG1  {25, 26};
  constexpr TL TL_REG2  {27, 28};
}
```

---

# 5) Lineamientos de codificación (resumen)

1. **Dueño único del hardware**: sólo la clase del dispositivo escribe sus pines.
2. **No bloqueante**: nada de `delay()`. Usa `Scheduler::every(ms, lambda)` o `update(now)` per-tick.
3. **Debounce** en todas las entradas discretas (botones/inductivos).
4. **Enums fuertes** (`enum class`) para estados/eventos; **nunca** números mágicos.
5. **Constantes/tiempos** en `core/Config.hpp` (fácil de tunear).
6. **Logs** con macros `LOG_INFO(...)`, `LOG_WARN(...)`, `LOG_ERR(...)` controladas por `LOG_LEVEL_*`.
7. **FSM con timeouts** → en caso de bloqueo, ir a `FAULT` con código de error.
8. **Documentación**: patrones híbridos documentados en `docs/lineamientos.md`.

---

# 6) Datos y reglas del dominio

## Tipos

```cpp
// core/Types.hpp
#pragma once
#include <stdint.h>

enum class VehicleClass : uint8_t { VIP, CARGA, REGULAR };
enum class SlotType : uint8_t { VIP, CARGA, REGULAR };
enum class SlotState : uint8_t { FREE, OCCUPIED };
```

## Reglas (FASE 1)

* Botón (clase) presionado → si hay **espacio disponible** para esa clase → **abrir barrera**.
* Al ocupar un slot: sensor ON → semáforo **ROJO**, libre = **VERDE**.
* Si **todo lleno** (6 ocupados) → no abrir barrera.
* Salida: botón salida + sensor seguro → abre barrera; al liberar (sensor OFF) → slot pasa a **FREE** y semáforo **VERDE**.

## Reglas (FASE 2) Prioridades

* Si **VIP lleno** pero hay **CARGA o REGULAR libres** → **VIP puede usar** uno de esos.
* Si **CARGA** o **REGULAR** están llenos y llega otro de su clase → **no entra** (no puede usar VIP).
* Algoritmo:

  1. Intentar asignar **slot de su misma clase**.
  2. Si clase = VIP y no hay, **permitir** CARGA o REGULAR (primero el que tenga más libres o política fija).
  3. Si clase = CARGA o REGULAR y su clase no tiene espacio → **rechazar**.

---

# 7) Dispositivos (POO)

## Barrera (servo + seguridad)

```cpp
// devices/Barrier.hpp
#pragma once
#include <Arduino.h>

class Barrier {
 public:
  void begin(uint8_t pwmPin);
  void setAngles(uint8_t closedDeg, uint8_t openDeg);
  void open();                         // comando
  void close();                        // comando
  void update(uint32_t nowMs, bool safeSensorActive);

  bool isMoving() const { return moving_; }
  bool isOpen()   const { return atOpen_; }
  bool isClosed() const { return atClosed_; }

 private:
  void moveTo(uint8_t targetDeg);

  uint8_t pin_{255};
  uint8_t openDeg_{90}, closedDeg_{0};
  uint8_t currentDeg_{0}, targetDeg_{0};
  bool    moving_{false}, atOpen_{false}, atClosed_{true};
  uint32_t lastStepMs_{0};
};
```

> Implementación: usa **ESP32Servo** (LEDC) y mueve en pequeños pasos cada `x ms` dentro de `update()` para que sea no bloqueante. **No cerrar** si `safeSensorActive` indica vehículo debajo.

## Semáforo por slot

```cpp
// devices/TrafficLight.hpp
#pragma once
#include <Arduino.h>

class TrafficLight {
public:
  void begin(uint8_t pinRed, uint8_t pinGreen) {
    pinMode(pinRed, OUTPUT); pinMode(pinGreen, OUTPUT);
    red_ = pinRed; green_ = pinGreen; setFree();
  }
  void setOccupied(){ digitalWrite(red_, HIGH); digitalWrite(green_, LOW); }
  void setFree()    { digitalWrite(red_, LOW);  digitalWrite(green_, HIGH); }

private:
  uint8_t red_{}, green_{};
};
```

## Sensor y botón (con debounce simple)

```cpp
// devices/ProximitySensor.hpp
#pragma once
#include <Arduino.h>

class ProximitySensor {
 public:
  void begin(uint8_t pin, bool pullup = true) {
    pin_ = pin; pinMode(pin_, pullup ? INPUT_PULLUP : INPUT);
  }
  bool read(uint32_t nowMs) {
    bool raw = digitalRead(pin_) == HIGH;           // ajusta si es NPN/PNP
    if (raw != lastRaw_) { lastChange_ = nowMs; lastRaw_ = raw; }
    if ((nowMs - lastChange_) > debounceMs_) stable_ = raw;
    return stable_;
  }
 private:
  uint8_t pin_{};
  bool lastRaw_{false}, stable_{false};
  uint32_t lastChange_{0};
  const uint16_t debounceMs_ = 30;
};
```

(Análogo para `Button` con pull-up y activo-bajo si lo necesitas.)

---

# 8) Gestión de slots y asignación (FASE 1/2)

```cpp
// app/SlotManager.hpp
#pragma once
#include <array>
#include "core/Types.hpp"
#include "devices/TrafficLight.hpp"

struct Slot {
  SlotType type;
  SlotState state;
  ProximitySensor sensor;
  TrafficLight tl;
};

class SlotManager {
public:
  static constexpr size_t kSlots = 6;
  void begin(/* pins config */);
  void update(uint32_t nowMs);

  // asignación según reglas FASE 1/2:
  int allocate(VehicleClass vc);   // retorna índice de slot o -1 si no hay
  void releaseByIndex(int idx);    // para liberar en salida

  // consulta
  bool allFull() const;
  size_t freeCount(SlotType t) const;

private:
  int findSameClass(VehicleClass vc) const;
  int findVipFallback() const; // VIP → CARGA o REGULAR

  std::array<Slot, kSlots> slots_;
};
```

**Idea de `allocate(vc)`**

* `vc=VIP`: intenta VIP; si no hay → `findVipFallback()` (CARGA o REGULAR).
* `vc=CARGA/REGULAR`: intenta su tipo; si no hay → `-1`.

**`update(nowMs)`** debe leer sensores: si sensor ON→`OCCUPIED` y semáforo rojo; OFF→`FREE` y semáforo verde.

---

# 9) Control de acceso (FSM de la barrera)

Eventos clave: `BtnVip`, `BtnCarga`, `BtnRegular`, `BtnExit`, `BarrierDone`, `Timeout`, `AllFullUpdate`.

```cpp
// app/AccessController.hpp
#pragma once
#include "core/Types.hpp"
#include "devices/Barrier.hpp"
#include "app/SlotManager.hpp"
#include "devices/Button.hpp"
#include "devices/ProximitySensor.hpp"

class AccessController {
public:
  enum class State : uint8_t { IDLE, CHECK_CAPACITY, OPENING, WAIT_PASS, CLOSING, FAULT };

  void begin(Barrier* barrier, SlotManager* slots,
             Button* btnVip, Button* btnCarga, Button* btnReg, Button* btnExit,
             ProximitySensor* safe);
  void update(uint32_t nowMs);

private:
  void requestEntry(VehicleClass vc);
  void requestExit();

  State state_{State::IDLE};
  Barrier* barrier_{nullptr};
  SlotManager* slots_{nullptr};
  Button *btnVip_{}, *btnCarga_{}, *btnReg_{}, *btnExit_{};
  ProximitySensor* safe_{};
  VehicleClass pending_{VehicleClass::REGULAR};
  int assignedSlot_{-1};
  uint32_t t0_{0};
  const uint32_t openTimeoutMs_ = 5000;
  const uint32_t passTimeMs_    = 3000;  // ventana para pasar (ajustable)
};
```

**Lógica resumida (no bloqueante):**

* `IDLE`: si `btnVip`/`btnCarga`/`btnReg` → `pending_` = clase; `state=CHECK_CAPACITY`. Si `btnExit` → `requestExit()`.
* `CHECK_CAPACITY`:

  * Si `slots_->allFull()` → rechazar (permanece `IDLE`).
  * Si `pending_==VIP` → `assignedSlot = slots_->allocate(VIP)` (con fallback a CARGA/REGULAR).
  * Si `CARGA/REGULAR` → `assignedSlot=allocate(clase)`; si `-1`, rechazar.
  * Si hay `assignedSlot>=0` → `OPENING` (comando `barrier->open()`).
* `OPENING`: vigila `safe` y `barrier->isOpen()` con timeout.
* `WAIT_PASS`: espera `passTimeMs_`; luego `CLOSING` (no cerrar si `safe` activo).
* `CLOSING`: `barrier->close()`, finaliza en `IDLE`.
* `FAULT`: requiere intervención (reset externo).

---

# 10) `src/main.cpp` (esqueleto)

```cpp
#include <Arduino.h>
#include "core/Scheduler.hpp"
#include "core/Logger.hpp"
#include "core/Pins.hpp"
#include "devices/Barrier.hpp"
#include "devices/ProximitySensor.hpp"
#include "devices/Button.hpp"
#include "app/SlotManager.hpp"
#include "app/AccessController.hpp"

Barrier barrier;
ProximitySensor safe;
Button btnVip, btnCarga, btnReg, btnExit;
SlotManager slots;
AccessController controller;

void setup() {
  Serial.begin(115200);
  LOG_INFO("Booting...");

  // Dispositivos
  barrier.begin(Pins::SERVO_PWM);
  barrier.setAngles(/*closed*/ 10, /*open*/ 90);

  safe.begin(Pins::BARRIER_SAFE_IN);

  btnVip.begin(Pins::BTN_VIP_IN, /*pullup=*/true, /*activeLow=*/true);
  btnCarga.begin(Pins::BTN_CARGA_IN, true, true);
  btnReg.begin(Pins::BTN_REG_IN, true, true);
  btnExit.begin(Pins::BTN_EXIT, true, true);

  slots.begin(); // inicializa sensores y semáforos de los 6 slots
  controller.begin(&barrier, &slots, &btnVip, &btnCarga, &btnReg, &btnExit, &safe);

  Scheduler::every(50, [](){  // 20Hz
    uint32_t now = millis();
    slots.update(now);
    controller.update(now);
    barrier.update(now, /*safeSensorActive=*/safe.read(now));
  });

  LOG_INFO("System ready.");
}

void loop() {
  Scheduler::tick(); // ejecuta tareas programadas no bloqueantes
}
```

> `Scheduler` es un mini-planificador: guarda lambdas con período y las ejecuta en `loop()` si toca. (Implementación simple en `core/Scheduler.hpp/.cpp`).

---

# 11) Configuración y parámetros (centralizados)

```cpp
// core/Config.hpp
#pragma once
#include <stdint.h>

namespace Cfg {
  // Tiempos ajustables
  constexpr uint32_t kPassTimeMs   = 3000;
  constexpr uint32_t kOpenTimeout  = 5000;

  // Debounces
  constexpr uint16_t kBtnDebounceMs = 30;
  constexpr uint16_t kSensDebounceMs = 30;

  // Política FASE 2: VIP fallback primero a CARGA, luego REGULAR
  enum class VipFallbackPolicy { CARGA_THEN_REGULAR, REGULAR_THEN_CARGA };
  constexpr VipFallbackPolicy kVipFallback = VipFallbackPolicy::CARGA_THEN_REGULAR;
}
```

---

# 12) Documentación y entregables

* **`docs/README.md`**:

  * Cómo instalar PlatformIO, compilar, flashear, abrir monitor
  * Esquema eléctrico de alto nivel
  * Instrucciones de pruebas (entradas simuladas, flujo FASE 1 y FASE 2)

* **`docs/Design.md`** (lineamientos):

  * Principio **dueño único** de pines por clase
  * FSMs con **timeouts** + **códigos de error**
  * **No-blocking**: uso de `Scheduler` y `update(now)`
  * **Estándar de nombres** (UpperCamel para clases, `kConst`, `snake_case` para pines)
  * **Eventos**: lista y tabla causa→efecto (matriz C\&E)
  * **Política FASE 2** bien definida (algoritmo de asignación)
  * **Documentación**: qué incluir en código (comentarios de business rules)

* **`docs/IO-Map.md`**:

  * Tabla con **pin físico**, **nombre lógico**, **dirección**, **tipo** (entrada/salida), **activo-alto/bajo**, **nota PNP/NPN**, **protección** (resistencias, opto, etc.)

---

# 13) Pruebas unitarias en PlatformIO

* Cubre **`SlotManager`**:

  * asignar cuando hay libres de su clase;
  * VIP usando fallback;
  * rechazos con clases llenas;
  * liberar y re-asignar.

(Validación directa en hardware ESP32-S3 para testing completo.)

---

# 14) Seguridad y hardware (apuntes rápidos)

* **Inductivos PNP/NPN**: valida niveles de 3V3 al ESP32S3 (usa **adaptación**: divisores, opto, o módulo conversor).
* **Relés**: aislar (opto y alimentación) si manejas cargas.
* **SG90**: alimenta por **5V** independiente y **GND común** con ESP32; evita alimentar servo del 3V3 del módulo.
* **Watchdog**: opcional activa Task WDT (FreeRTOS) si más adelante migras a `esp-idf` o usas wrappers Arduino.

---

# 15) ¿Por qué híbrido y no 100% estructurado u 100% POO?

* 100% estructurado: rápido, pero **escala peor**; mezclar IO disperso en FSMs produce deuda técnica.
* 100% POO: elegante, pero **FSMs escondidas** dentro de objetos grandes se vuelven opacas para el comisionamiento.
* **Híbrido**: dispositivos **POO** (interfaces claras) + **FSMs explícitas** (legibles, mantenibles) = **robustez + mantenibilidad**.

---


Con eso puedes enchufar pines reales y empezar a probar en mesa sin dolores.
