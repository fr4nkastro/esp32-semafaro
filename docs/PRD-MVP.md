# PRD - SEMAFARO MVP
## Product Requirements Document - Sistema de Control de Estacionamiento

### Información del Documento
- **Fecha**: 21 de Septiembre, 2025
- **Versión**: 1.0
- **Proyecto**: SEMAFARO - Sistema de Control de Acceso para Estacionamiento
- **Autor**: GitHub Copilot
- **Estado**: Draft

---

## 1. Resumen Ejecutivo

### 1.1 Visión del Producto
Desarrollar un sistema automatizado de control de acceso para estacionamiento con capacidad para 6 vehículos, divididos en 3 categorías (VIP, Carga, Regular), utilizando ESP32-S3, sensores inductivos, semáforos LED y servo para barrera.

### 1.2 Objetivos del MVP
- Demostrar funcionalidad básica de control de acceso (FASE 1)
- Validar arquitectura técnica híbrida (POO + FSM)
- Probar integración hardware-software
- Establecer base para funcionalidades avanzadas (FASE 2)

### 1.3 Valor de Negocio
- **Automatización**: Reducir intervención manual en control de acceso
- **Eficiencia**: Optimizar uso de espacios de estacionamiento
- **Escalabilidad**: Base técnica sólida para futuras expansiones
- **Costo-efectividad**: Solución basada en hardware accesible

---

## 2. Alcance del MVP

### 2.1 Funcionalidades Incluidas (FASE 1 Completa)

#### 2.1.1 Control de Acceso Básico
- **RF-001**: Tres botoneras de entrada (VIP, Carga, Regular)
- **RF-002**: Una botonera de salida
- **RF-003**: Apertura automática de barrera según disponibilidad
- **RF-004**: Sensor de seguridad anti-aplastamiento
- **RF-005**: Bloqueo de acceso cuando estacionamiento lleno

#### 2.1.2 Gestión de Espacios
- **RF-006**: 6 espacios divididos en: 2 VIP, 2 Carga, 2 Regular
- **RF-007**: Sensores inductivos en cada espacio
- **RF-008**: Semáforos LED por espacio (Rojo=Ocupado, Verde=Libre)
- **RF-009**: Detección automática de ocupación/liberación

#### 2.1.3 Funcionalidades Básicas FASE 2
- **RF-010**: Prioridad VIP para usar espacios Carga/Regular cuando VIP lleno
- **RF-011**: Restricción de acceso para Carga/Regular cuando sus espacios están llenos

### 2.2 Funcionalidades Excluidas del MVP
- Interface web/móvil
- Comunicación WiFi/Bluetooth
- Base de datos de vehículos
- Reportes y analytics
- Integración con sistemas de pago
- Reconocimiento de placas

---

## 3. Requerimientos Funcionales Detallados

### 3.1 Control de Barrera (AccessController)

| ID | Requerimiento | Prioridad | Criterio de Aceptación |
|---|---|---|---|
| RF-001 | Apertura por botonera | Alta | Al presionar botón válido, barrera abre en <2s si hay espacio disponible |
| RF-002 | Sensor anti-aplastamiento | Crítica | Barrera no cierra si sensor detecta obstáculo |
| RF-003 | Timeout de apertura | Media | Barrera cierra automáticamente después de 5s sin actividad |
| RF-004 | Manejo de errores | Alta | Sistema entra en modo FAULT si hay timeout o error de hardware |

### 3.2 Gestión de Espacios (SlotManager)

| ID | Requerimiento | Prioridad | Criterio de Aceptación |
|---|---|---|---|
| RF-005 | Detección de ocupación | Alta | Sensor inductivo actualiza estado en <500ms |
| RF-006 | Control de semáforos | Alta | LED rojo cuando ocupado, verde cuando libre |
| RF-007 | Asignación por clase | Alta | Vehículo dirigido al espacio correcto según botón presionado |
| RF-008 | Prioridad VIP | Media | VIP puede usar Carga/Regular si VIP lleno |

### 3.3 Sistema de Seguridad

| ID | Requerimiento | Prioridad | Criterio de Aceptación |
|---|---|---|---|
| RF-009 | Debounce de sensores | Alta | Filtrar ruido eléctrico con debounce de 30ms |
| RF-010 | Validación de estados | Media | Verificar consistencia sensor-semáforo cada 100ms |
| RF-011 | Recuperación de fallas | Alta | Reset automático de estados inconsistentes |

---

## 4. Requerimientos No Funcionales

### 4.1 Performance
- **NF-001**: Tiempo de respuesta <500ms para cualquier acción
- **NF-002**: Actualización de estados cada 50ms (20Hz)
- **NF-003**: Movimiento de barrera suave (paso a paso, no bloqueante)

### 4.2 Confiabilidad
- **NF-004**: Sistema operativo 24/7 sin reinicio manual
- **NF-005**: Recuperación automática de errores no críticos
- **NF-006**: Tolerancia a ruido eléctrico (debounce, filtros)

### 4.3 Mantenibilidad
- **NF-007**: Código modular siguiendo lineamientos híbridos
- **NF-008**: Logs detallados con niveles INFO/WARN/ERROR
- **NF-009**: Configuración centralizada en archivos de header

### 4.4 Usabilidad
- **NF-010**: Feedback visual inmediato (LEDs, semáforos)
- **NF-011**: Operación intuitiva (un botón por tipo de vehículo)

---

## 5. Arquitectura Técnica

### 5.1 Hardware Requerido

#### 5.1.1 Componentes Principales
| Componente | Especificación | Cantidad | Función |
|---|---|---|---|
| ESP32-S3 | N16R8 16MB Flash, USB-C | 1 | Controlador principal |
| Servo SG90 | 180° 1.5Kg | 1 | Barrera |
| Sensores Inductivos | SN04-N 10-30V 4mm | 7 | Detección vehículos + seguridad |
| Módulos LED Semáforo | 10mm Rojo/Verde | 6 | Indicadores por espacio |
| Botoneras | 4 canales | 1 | Control entrada/salida |
| Módulo Relés | 8 canales 5V | 1 | Control semáforos |
| Fuente | 3.3V/5V 700mAh | 1 | Alimentación |

#### 5.1.2 Asignación de Pines (Propuesta)
```cpp
// Servo y seguridad
SERVO_PWM = 5
BARRIER_SAFE_IN = 10

// Botoneras
BTN_VIP_IN = 6, BTN_CARGA_IN = 7, BTN_REG_IN = 8, BTN_EXIT = 9

// Sensores de espacios
S_VIP1 = 11, S_VIP2 = 12, S_CARG1 = 13, S_CARG2 = 14, S_REG1 = 15, S_REG2 = 16

// Semáforos (ejemplo usando relés)
TL_VIP1 = {17,18}, TL_VIP2 = {19,20}, etc.
```

### 5.2 Arquitectura de Software

#### 5.2.1 Estructura Híbrida
- **Dispositivos (POO)**: Encapsulación de hardware (Barrier, TrafficLight, ProximitySensor)
- **Lógica de Control (FSM)**: Máquinas de estado para flujos complejos
- **Coordinación**: SlotManager + AccessController
- **Framework**: Arduino sobre ESP32-S3

#### 5.2.2 Componentes Principales
```
src/
├── main.cpp                    // Punto de entrada y setup
├── devices/                    // Clases POO para hardware
│   ├── Barrier.hpp/.cpp
│   ├── TrafficLight.hpp/.cpp
│   ├── ProximitySensor.hpp/.cpp
│   └── Button.hpp/.cpp
├── app/                        // Lógica de aplicación
│   ├── AccessController.hpp/.cpp   // FSM de barrera
│   ├── SlotManager.hpp/.cpp        // Gestión de espacios
│   └── Events.hpp                  // Definición de eventos
└── core/                       // Infraestructura
    ├── Scheduler.hpp/.cpp          // Tareas no bloqueantes
    ├── Logger.hpp                  // Sistema de logs
    ├── Pins.hpp                    // Mapeo de pines
    └── Config.hpp                  // Configuración
```

#### 5.2.3 Máquina de Estados Principal (AccessController)
```
IDLE → CHECK_CAPACITY → OPENING → WAIT_PASS → CLOSING → IDLE
                    ↓
                  FAULT (recuperación manual)
```

---

## 6. Plan de Implementación

### 6.1 Fases de Desarrollo

#### Fase 1: Infraestructura (Sprint 1 )
- [ ] Setup proyecto PlatformIO
- [ ] Implementar clases base de dispositivos
- [ ] Sistema de logging y scheduler
- [ ] Mapeo de pines y configuración

#### Fase 2: Dispositivos Básicos (Sprint 2 )
- [ ] Clase Barrier con servo
- [ ] Clases Button y ProximitySensor con debounce
- [ ] Clase TrafficLight
- [ ] Pruebas unitarias de dispositivos

#### Fase 3: Gestión de Espacios (Sprint 3 )
- [ ] SlotManager completo
- [ ] Lógica de asignación FASE 1
- [ ] Integración con semáforos
- [ ] Pruebas de asignación

#### Fase 4: Control de Acceso (Sprint 4 )
- [ ] AccessController FSM
- [ ] Integración barrera-sensores
- [ ] Manejo de timeouts y errores
- [ ] Pruebas de flujo completo

#### Fase 5: Funcionalidades FASE 2 (Sprint 5 )
- [ ] Prioridad VIP
- [ ] Restricciones de acceso
- [ ] Optimización y ajustes
- [ ] Pruebas de integración final

#### Fase 6: Validación y Documentación (Sprint 6 )
- [ ] Pruebas de hardware completo
- [ ] Documentación técnica
- [ ] Manual de usuario básico
- [ ] Preparación para demo

### 6.2 Dependencias y Riesgos

#### Dependencias Críticas
- Disponibilidad de componentes hardware
- Configuración correcta de PlatformIO
- Validación de niveles de voltaje (3.3V vs 5V)

#### Riesgos Identificados
| Riesgo | Probabilidad | Impacto | Mitigación |
|---|---|---|---|
| Incompatibilidad sensores-ESP32 | Media | Alto | Prototipar conexiones temprano |
| Problemas de alimentación | Baja | Alto | Usar fuente externa para servo |
| Ruido eléctrico | Alta | Medio | Implementar debounce robusto |
| Complejidad FSM | Media | Medio | Desarrollo incremental |

---

## 7. Criterios de Aceptación del MVP

### 7.1 Funcionalidades Core
- [ ] **Acceso básico**: Botón → Barrera abre (si hay espacio) → Vehículo pasa → Barrera cierra
- [ ] **Ocupación**: Sensor detecta vehículo → Semáforo rojo → Espacio marcado ocupado
- [ ] **Liberación**: Vehículo sale → Sensor libera → Semáforo verde → Espacio disponible
- [ ] **Seguridad**: Sensor anti-aplastamiento funciona correctamente
- [ ] **Capacidad**: Sistema rechaza acceso cuando está lleno (6 espacios ocupados)

### 7.2 Escenarios de Prueba
1. **Entrada normal**: VIP presiona botón, hay espacio VIP libre → acceso permitido
2. **Estacionamiento lleno**: Cualquier botón presionado → acceso denegado
3. **Prioridad VIP**: VIP presiona botón, VIP lleno pero hay Carga libre → acceso a Carga
4. **Restricción Regular**: Regular presiona botón, Regular lleno → acceso denegado
5. **Seguridad**: Objeto en barrera → barrera no cierra
6. **Salida**: Botón salida → barrera abre → espacio se libera al salir


## 8. Plan de Pruebas

### 8.1 Pruebas Unitarias
- Dispositivos individuales (servo, sensores, botones)
- Lógica de asignación SlotManager
- Estados de AccessController

### 8.2 Pruebas de Integración
- Comunicación entre componentes
- Secuencias completas de entrada/salida
- Manejo de errores y timeouts

### 8.3 Pruebas de Sistema
- Escenarios de uso completos
- Pruebas de estrés (múltiples activaciones)
- Validación de seguridad



---

## 9. Entregables

### 9.1 Software
- [ ] Código fuente completo en PlatformIO
- [ ] Documentación técnica de la API
- [ ] Casos de prueba automatizados
- [ ] Manual de configuración

### 9.2 Hardware
- [ ] Prototipo funcional ensamblado
- [ ] Esquema de conexiones
- [ ] Lista de materiales (BOM)
- [ ] Guía de montaje

### 9.3 Documentación
- [ ] Manual de usuario
- [ ] Guía de troubleshooting
- [ ] Documentación de arquitectura


---


## 11. Conclusiones

Este PRD define un MVP robusto que valida los conceptos core del sistema SEMAFARO mientras establece una base técnica sólida para futuras expansiones. La arquitectura híbrida propuesta balanceará simplicidad de desarrollo con mantenibilidad a largo plazo.

**Criterio de éxito principal**: Demostración exitosa de control de acceso automatizado con las 6 funcionalidades core implementadas y probadas.