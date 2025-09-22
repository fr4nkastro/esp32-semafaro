# SEMAFARO MVP - Implementación Inicial

## FEATURE
- **Purpose**: Implementar sistema automatizado de control de acceso para estacionamiento con 6 espacios divididos en 3 categorías (VIP, Carga, Regular)
- **Stakeholders**: Desarrollador principal, usuario final del sistema de estacionamiento
- **Functional Requirements**: 
  - Control de acceso con 4 botoneras (VIP, Carga, Regular, Salida)
  - Gestión de 6 espacios con sensores inductivos y semáforos LED
  - Barrera servo con sensor anti-aplastamiento
  - Prioridad VIP para usar espacios Carga/Regular
- **Non-Functional Requirements**: 
  - Tiempo de respuesta <500ms
  - Operación 24/7 sin reinicio manual
  - Arquitectura híbrida POO + FSM
- **Architecture Approach**: ESP32-S3 con arquitectura híbrida (POO para dispositivos, FSM para control)

## EXAMPLES & DOCUMENTATION  
- **Code References**: Lineamientos híbridos en `/docs/lineamientos.md`
- **API Documentation**: ESP32Servo library, Arduino framework
- **Design Patterns**: 
  - Scheduler pattern para tareas no bloqueantes
  - State Machine pattern para AccessController
  - Observer pattern para eventos de sensores
- **Anti-patterns**: Uso de delay(), bloqueo de hardware, acceso directo a pines

## TECHNICAL SPECIFICATIONS
- **Data Models**: 
  - VehicleClass enum (VIP, CARGA, REGULAR)
  - SlotState enum (FREE, OCCUPIED)
  - AccessState enum (IDLE, CHECK_CAPACITY, OPENING, WAIT_PASS, CLOSING, FAULT)
- **API Contracts**: 
  - Device classes con begin() y update(nowMs) patterns
  - Event-driven communication entre componentes
- **Integration Points**: 
  - ESP32Servo library para control de barrera
  - GPIO directo para sensores y LEDs
  - Scheduler para coordinación temporal
- **Testing Strategy**: 
  - Unit tests para SlotManager allocation logic
  - Hardware validation con prototipos
  - Integration tests para flujos completos

## OTHER CONSIDERATIONS
- **Security Requirements**: Debounce robusto para ruido eléctrico, validación de estados
- **Performance Constraints**: Update rate 20Hz (50ms), response time <500ms
- **Risk Assessment**: 
  - Incompatibilidad voltajes 3.3V/5V - usar level shifters
  - Ruido eléctrico - implementar debounce 30ms
  - Complejidad FSM - desarrollo incremental
- **Compliance**: Seguir lineamientos híbridos y patrones no-bloqueantes

## IMPLEMENTATION PHASES
1. **Infraestructura Base**: Scheduler, Logger, Pins, Config
2. **Dispositivos Hardware**: Barrier, TrafficLight, ProximitySensor, Button
3. **Gestión Espacios**: SlotManager con lógica de asignación
4. **Control Acceso**: AccessController FSM
5. **Funcionalidades Avanzadas**: Prioridad VIP, restricciones
6. **Validación Final**: Testing integrado y documentación