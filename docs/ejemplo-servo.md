# Guía de Implementación: Sistema de Control de Barrera Vehicular para Parqueo

## Análisis del Código Base Existente

El proyecto actual contiene un sistema de control de peso con servomotor que incluye:
- Control PWM de servomotor SG90 (pin 14)
- Sistema de pesaje con HX711
- Comunicación serial para comandos
- Multitareas con FreeRTOS

### Funciones de Servomotor Disponibles:
```cpp
void setGate(int grados)          // Controla posición del servo (0-180°)
servoMotor.attach(pin)           // Configura el servo en un pin
servoMotor.writeMicroseconds()   // Control preciso en microsegundos
servoMotor.read()                // Lee posición actual en grados
```

## Componentes del Sistema de Parqueo

### Hardware Requerido:
- **ESP32-S3** (controlador principal)
- **Servomotor SG90** (barrera vehicular)
- **Módulo de relés 8 canales** (control de semáforos)
- **6 Módulos de semáforo LED** (indicadores de estado)
- **7 Sensores inductivos** (detección de vehículos)
- **Botonera cuádruple** (tarjetas de acceso simuladas)
- **Fuente de alimentación 3.3V/5V**

### Distribución de Pines ESP32:

```cpp
// Servomotor (Barrera)
#define SERVO_PIN 14

// Módulo de Relés (Semáforos)
#define RELAY_1 2   // Semáforo VIP 1
#define RELAY_2 4   // Semáforo VIP 2
#define RELAY_3 5   // Semáforo Carga 1
#define RELAY_4 18  // Semáforo Carga 2
#define RELAY_5 19  // Semáforo Regular 1
#define RELAY_6 21  // Semáforo Regular 2

// Sensores Inductivos (Espacios de Parqueo)
#define SENSOR_VIP_1    23
#define SENSOR_VIP_2    22
#define SENSOR_CARGA_1  16
#define SENSOR_CARGA_2  17
#define SENSOR_REGULAR_1 25
#define SENSOR_REGULAR_2 26

// Sensor de Barrera (Evita caída sobre vehículo)
#define SENSOR_BARRERA  27

// Botones de Acceso
#define BTN_VIP      32
#define BTN_CARGA    33
#define BTN_REGULAR  34
#define BTN_SALIDA   35
```

## Implementación Paso a Paso

### Paso 1: Configuración Inicial del Hardware

```cpp
#include <Arduino.h>
#include <ESP32Servo.h>

// Definición de pines
#define SERVO_PIN 14
#define SENSOR_BARRERA 27

// Arrays para manejo de semáforos y sensores
int relayPins[] = {2, 4, 5, 18, 19, 21};
int sensorPins[] = {23, 22, 16, 17, 25, 26};
int buttonPins[] = {32, 33, 34, 35};

// Estados del sistema
bool espaciosOcupados[6] = {false, false, false, false, false, false};
bool barreraAbierta = false;
int tipoVehiculoEntrando = -1; // 0=VIP, 1=CARGA, 2=REGULAR

Servo servoBarrera;

void setup() {
    Serial.begin(115200);
    
    // Configurar servo
    servoBarrera.attach(SERVO_PIN);
    cerrarBarrera();
    
    // Configurar relés como salidas
    for(int i = 0; i < 6; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], HIGH); // LED verde (espacio libre)
    }
    
    // Configurar sensores como entradas
    for(int i = 0; i < 6; i++) {
        pinMode(sensorPins[i], INPUT);
    }
    pinMode(SENSOR_BARRERA, INPUT);
    
    // Configurar botones como entradas con pull-up
    for(int i = 0; i < 4; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
}
```

### Paso 2: Funciones de Control de Barrera

```cpp
void abrirBarrera() {
    servoBarrera.write(90); // Abre la barrera
    barreraAbierta = true;
    Serial.println("Barrera abierta");
}

void cerrarBarrera() {
    servoBarrera.write(0);  // Cierra la barrera
    barreraAbierta = false;
    Serial.println("Barrera cerrada");
}

bool vehiculoEnBarrera() {
    return digitalRead(SENSOR_BARRERA) == HIGH;
}

void manejarBarrera() {
    if (barreraAbierta && !vehiculoEnBarrera()) {
        delay(2000); // Espera 2 segundos después de que pase el vehículo
        cerrarBarrera();
    }
}
```

### Paso 3: Sistema de Gestión de Espacios

```cpp
enum TipoVehiculo { VIP = 0, CARGA = 1, REGULAR = 2 };

struct EspacioParqueo {
    int espaciosVIP[2] = {0, 1};
    int espaciosCarga[2] = {2, 3};
    int espaciosRegular[2] = {4, 5};
};

EspacioParqueo parqueo;

void actualizarSemaforo(int espacio, bool ocupado) {
    if (ocupado) {
        digitalWrite(relayPins[espacio], LOW);  // LED rojo
        Serial.println("Espacio " + String(espacio) + " ocupado");
    } else {
        digitalWrite(relayPins[espacio], HIGH); // LED verde
        Serial.println("Espacio " + String(espacio) + " libre");
    }
}

bool hayEspacioDisponible(TipoVehiculo tipo) {
    switch(tipo) {
        case VIP:
            return !espaciosOcupados[0] || !espaciosOcupados[1];
        case CARGA:
            return !espaciosOcupados[2] || !espaciosOcupados[3];
        case REGULAR:
            return !espaciosOcupados[4] || !espaciosOcupados[5];
    }
    return false;
}

int encontrarEspacioLibre(TipoVehiculo tipo) {
    switch(tipo) {
        case VIP:
            if (!espaciosOcupados[0]) return 0;
            if (!espaciosOcupados[1]) return 1;
            break;
        case CARGA:
            if (!espaciosOcupados[2]) return 2;
            if (!espaciosOcupados[3]) return 3;
            break;
        case REGULAR:
            if (!espaciosOcupados[4]) return 4;
            if (!espaciosOcupados[5]) return 5;
            break;
    }
    return -1;
}
```

### Paso 4: Lógica de Prioridades (FASE 2)

```cpp
bool puedeIngresarVehiculo(TipoVehiculo tipo) {
    // FASE 1: Verificar espacios del mismo tipo
    if (hayEspacioDisponible(tipo)) {
        return true;
    }
    
    // FASE 2: Lógica de prioridades para VIP
    if (tipo == VIP) {
        // VIP puede usar espacios de CARGA o REGULAR si están disponibles
        return hayEspacioDisponible(CARGA) || hayEspacioDisponible(REGULAR);
    }
    
    // CARGA y REGULAR no pueden usar otros tipos si están llenos
    return false;
}

int asignarEspacio(TipoVehiculo tipo) {
    // Primero intentar espacios del mismo tipo
    int espacio = encontrarEspacioLibre(tipo);
    if (espacio != -1) {
        return espacio;
    }
    
    // Si es VIP y no hay espacios VIP, buscar en otros tipos
    if (tipo == VIP) {
        espacio = encontrarEspacioLibre(CARGA);
        if (espacio != -1) return espacio;
        
        espacio = encontrarEspacioLibre(REGULAR);
        if (espacio != -1) return espacio;
    }
    
    return -1; // No hay espacios disponibles
}
```

### Paso 5: Manejo de Sensores y Botones

```cpp
void leerSensoresParqueo() {
    for(int i = 0; i < 6; i++) {
        bool estadoAnterior = espaciosOcupados[i];
        espaciosOcupados[i] = digitalRead(sensorPins[i]) == HIGH;
        
        // Si cambió el estado, actualizar semáforo
        if (estadoAnterior != espaciosOcupados[i]) {
            actualizarSemaforo(i, espaciosOcupados[i]);
        }
    }
}

void procesarBotones() {
    // Botón VIP
    if (digitalRead(buttonPins[0]) == LOW) {
        if (puedeIngresarVehiculo(VIP)) {
            tipoVehiculoEntrando = VIP;
            abrirBarrera();
        } else {
            Serial.println("No hay espacios VIP disponibles");
        }
        delay(500); // Debounce
    }
    
    // Botón CARGA
    if (digitalRead(buttonPins[1]) == LOW) {
        if (puedeIngresarVehiculo(CARGA)) {
            tipoVehiculoEntrando = CARGA;
            abrirBarrera();
        } else {
            Serial.println("No hay espacios CARGA disponibles");
        }
        delay(500);
    }
    
    // Botón REGULAR
    if (digitalRead(buttonPins[2]) == LOW) {
        if (puedeIngresarVehiculo(REGULAR)) {
            tipoVehiculoEntrando = REGULAR;
            abrirBarrera();
        } else {
            Serial.println("No hay espacios REGULAR disponibles");
        }
        delay(500);
    }
    
    // Botón SALIDA
    if (digitalRead(buttonPins[3]) == LOW) {
        abrirBarrera();
        delay(500);
    }
}
```

### Paso 6: Loop Principal

```cpp
void loop() {
    leerSensoresParqueo();
    procesarBotones();
    manejarBarrera();
    
    // Mostrar estado del sistema cada 5 segundos
    static unsigned long ultimoReporte = 0;
    if (millis() - ultimoReporte > 5000) {
        mostrarEstadoSistema();
        ultimoReporte = millis();
    }
    
    delay(100);
}

void mostrarEstadoSistema() {
    Serial.println("=== Estado del Parqueo ===");
    Serial.println("VIP: " + String(!espaciosOcupados[0] ? "LIBRE" : "OCUPADO") + 
                   " | " + String(!espaciosOcupados[1] ? "LIBRE" : "OCUPADO"));
    Serial.println("CARGA: " + String(!espaciosOcupados[2] ? "LIBRE" : "OCUPADO") + 
                   " | " + String(!espaciosOcupados[3] ? "LIBRE" : "OCUPADO"));
    Serial.println("REGULAR: " + String(!espaciosOcupados[4] ? "LIBRE" : "OCUPADO") + 
                   " | " + String(!espaciosOcupados[5] ? "LIBRE" : "OCUPADO"));
    Serial.println("Barrera: " + String(barreraAbierta ? "ABIERTA" : "CERRADA"));
    Serial.println("========================");
}
```

## Ejemplo Completo del Sistema

### Código Principal Completo:

```cpp
#include <Arduino.h>
#include <ESP32Servo.h>

// Definición de pines
#define SERVO_PIN 14
#define SENSOR_BARRERA 27

// Arrays para manejo de componentes
int relayPins[] = {2, 4, 5, 18, 19, 21};        // Pines de relés para semáforos
int sensorPins[] = {23, 22, 16, 17, 25, 26};    // Pines de sensores de parqueo
int buttonPins[] = {32, 33, 34, 35};            // Pines de botones (VIP, CARGA, REGULAR, SALIDA)

// Estados del sistema
bool espaciosOcupados[6] = {false, false, false, false, false, false};
bool barreraAbierta = false;
int tipoVehiculoEntrando = -1;

// Enum para tipos de vehículo
enum TipoVehiculo { VIP = 0, CARGA = 1, REGULAR = 2 };

Servo servoBarrera;

// Funciones de control de barrera
void abrirBarrera() {
    servoBarrera.write(90);
    barreraAbierta = true;
    Serial.println("🚧 Barrera ABIERTA");
}

void cerrarBarrera() {
    servoBarrera.write(0);
    barreraAbierta = false;
    Serial.println("🚧 Barrera CERRADA");
}

bool vehiculoEnBarrera() {
    return digitalRead(SENSOR_BARRERA) == HIGH;
}

// Funciones de gestión de espacios
void actualizarSemaforo(int espacio, bool ocupado) {
    if (ocupado) {
        digitalWrite(relayPins[espacio], LOW);   // LED rojo
        Serial.println("🔴 Espacio " + String(espacio + 1) + " OCUPADO");
    } else {
        digitalWrite(relayPins[espacio], HIGH);  // LED verde
        Serial.println("🟢 Espacio " + String(espacio + 1) + " LIBRE");
    }
}

bool hayEspacioDisponible(TipoVehiculo tipo) {
    switch(tipo) {
        case VIP:
            return !espaciosOcupados[0] || !espaciosOcupados[1];
        case CARGA:
            return !espaciosOcupados[2] || !espaciosOcupados[3];
        case REGULAR:
            return !espaciosOcupados[4] || !espaciosOcupados[5];
    }
    return false;
}

int encontrarEspacioLibre(TipoVehiculo tipo) {
    switch(tipo) {
        case VIP:
            if (!espaciosOcupados[0]) return 0;
            if (!espaciosOcupados[1]) return 1;
            break;
        case CARGA:
            if (!espaciosOcupados[2]) return 2;
            if (!espaciosOcupados[3]) return 3;
            break;
        case REGULAR:
            if (!espaciosOcupados[4]) return 4;
            if (!espaciosOcupados[5]) return 5;
            break;
    }
    return -1;
}

// Lógica de prioridades (FASE 2)
bool puedeIngresarVehiculo(TipoVehiculo tipo) {
    // Verificar espacios del mismo tipo
    if (hayEspacioDisponible(tipo)) {
        return true;
    }
    
    // VIP puede usar espacios de CARGA o REGULAR
    if (tipo == VIP) {
        return hayEspacioDisponible(CARGA) || hayEspacioDisponible(REGULAR);
    }
    
    return false;
}

int asignarEspacio(TipoVehiculo tipo) {
    int espacio = encontrarEspacioLibre(tipo);
    if (espacio != -1) return espacio;
    
    // Si es VIP, buscar en otros tipos
    if (tipo == VIP) {
        espacio = encontrarEspacioLibre(CARGA);
        if (espacio != -1) return espacio;
        
        espacio = encontrarEspacioLibre(REGULAR);
        if (espacio != -1) return espacio;
    }
    
    return -1;
}

// Lectura de sensores
void leerSensoresParqueo() {
    for(int i = 0; i < 6; i++) {
        bool estadoAnterior = espaciosOcupados[i];
        espaciosOcupados[i] = digitalRead(sensorPins[i]) == HIGH;
        
        if (estadoAnterior != espaciosOcupados[i]) {
            actualizarSemaforo(i, espaciosOcupados[i]);
        }
    }
}

// Procesamiento de botones
void procesarBotones() {
    static unsigned long ultimaLectura = 0;
    if (millis() - ultimaLectura < 500) return; // Debounce
    
    // Botón VIP
    if (digitalRead(buttonPins[0]) == LOW) {
        if (puedeIngresarVehiculo(VIP)) {
            Serial.println("🅿️ Acceso VIP autorizado");
            tipoVehiculoEntrando = VIP;
            abrirBarrera();
        } else {
            Serial.println("❌ Sin espacios VIP disponibles");
        }
        ultimaLectura = millis();
    }
    
    // Botón CARGA
    else if (digitalRead(buttonPins[1]) == LOW) {
        if (puedeIngresarVehiculo(CARGA)) {
            Serial.println("🚛 Acceso CARGA autorizado");
            tipoVehiculoEntrando = CARGA;
            abrirBarrera();
        } else {
            Serial.println("❌ Sin espacios CARGA disponibles");
        }
        ultimaLectura = millis();
    }
    
    // Botón REGULAR
    else if (digitalRead(buttonPins[2]) == LOW) {
        if (puedeIngresarVehiculo(REGULAR)) {
            Serial.println("🚗 Acceso REGULAR autorizado");
            tipoVehiculoEntrando = REGULAR;
            abrirBarrera();
        } else {
            Serial.println("❌ Sin espacios REGULAR disponibles");
        }
        ultimaLectura = millis();
    }
    
    // Botón SALIDA
    else if (digitalRead(buttonPins[3]) == LOW) {
        Serial.println("🚪 Vehículo saliendo");
        abrirBarrera();
        ultimaLectura = millis();
    }
}

// Manejo de barrera
void manejarBarrera() {
    static unsigned long tiempoVehiculoPaso = 0;
    
    if (barreraAbierta) {
        if (vehiculoEnBarrera()) {
            tiempoVehiculoPaso = millis(); // Resetear timer mientras hay vehículo
        } else if (millis() - tiempoVehiculoPaso > 3000) { // 3 segundos después
            cerrarBarrera();
        }
    }
}

// Mostrar estado del sistema
void mostrarEstadoSistema() {
    Serial.println("\n📊 === Estado del Sistema de Parqueo ===");
    Serial.println("🅿️  VIP:     [" + String(!espaciosOcupados[0] ? "🟢" : "🔴") + "] [" + String(!espaciosOcupados[1] ? "🟢" : "🔴") + "]");
    Serial.println("🚛 CARGA:   [" + String(!espaciosOcupados[2] ? "🟢" : "🔴") + "] [" + String(!espaciosOcupados[3] ? "🟢" : "🔴") + "]");
    Serial.println("🚗 REGULAR: [" + String(!espaciosOcupados[4] ? "🟢" : "🔴") + "] [" + String(!espaciosOcupados[5] ? "🟢" : "🔴") + "]");
    Serial.println("🚧 Barrera: " + String(barreraAbierta ? "ABIERTA" : "CERRADA"));
    
    // Calcular espacios libres por tipo
    int libresVIP = (!espaciosOcupados[0] ? 1 : 0) + (!espaciosOcupados[1] ? 1 : 0);
    int libresCarga = (!espaciosOcupados[2] ? 1 : 0) + (!espaciosOcupados[3] ? 1 : 0);
    int libresRegular = (!espaciosOcupados[4] ? 1 : 0) + (!espaciosOcupados[5] ? 1 : 0);
    
    Serial.println("📈 Disponibilidad: VIP(" + String(libresVIP) + "/2) CARGA(" + String(libresCarga) + "/2) REGULAR(" + String(libresRegular) + "/2)");
    Serial.println("=====================================\n");
}

void setup() {
    Serial.begin(115200);
    Serial.println("🚀 Iniciando Sistema de Control de Parqueo...");
    
    // Configurar servo
    servoBarrera.attach(SERVO_PIN);
    cerrarBarrera();
    
    // Configurar relés (semáforos)
    for(int i = 0; i < 6; i++) {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], HIGH); // Iniciar en verde (libre)
    }
    
    // Configurar sensores
    for(int i = 0; i < 6; i++) {
        pinMode(sensorPins[i], INPUT);
    }
    pinMode(SENSOR_BARRERA, INPUT);
    
    // Configurar botones
    for(int i = 0; i < 4; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
    
    Serial.println("✅ Sistema inicializado correctamente");
    mostrarEstadoSistema();
}

void loop() {
    leerSensoresParqueo();
    procesarBotones();
    manejarBarrera();
    
    // Reporte periódico del estado
    static unsigned long ultimoReporte = 0;
    if (millis() - ultimoReporte > 10000) { // Cada 10 segundos
        mostrarEstadoSistema();
        ultimoReporte = millis();
    }
    
    delay(50);
}
```

## Configuración de PlatformIO

Actualizar `platformio.ini`:

```ini
[env:upesy_wroom]
platform = espressif32
board = upesy_wroom
framework = arduino
monitor_speed = 115200
lib_deps = 
    madhephaestus/ESP32Servo@^0.13.0
    bogde/HX711@^0.7.5
```

## Diagrama de Conexiones

```
ESP32-S3 Conexiones:
├── GPIO14  → Servomotor (PWM)
├── GPIO27  → Sensor Barrera
├── GPIO2-21 → Módulo Relés (Semáforos)
├── GPIO23-26 → Sensores Parqueo
└── GPIO32-35 → Botones Acceso

Alimentación:
├── 5V → Servomotor, Relés, Sensores
├── 3.3V → ESP32, Botones
└── GND → Común a todos
```

## Características del Sistema

### ✅ Funcionalidades Implementadas:

1. **Control de Barrera PWM** - Apertura/cierre preciso
2. **Detección de Vehículos** - Sensores inductivos
3. **Sistema de Semáforos** - Indicación visual de espacios
4. **Lógica de Prioridades** - VIP puede usar cualquier espacio
5. **Seguridad** - Sensor evita caída de barrera
6. **Monitoreo Serial** - Estado en tiempo real

### 🔧 Características Técnicas:

- **Microcontrolador**: ESP32-S3
- **Comunicación**: UART 115200 bps
- **Control PWM**: Servo 180° (SG90)
- **Sensores**: Inductivos PNP/NPN
- **Capacidad**: 6 vehículos (2 VIP, 2 Carga, 2 Regular)

### 📋 Comandos Serial Disponibles:

- `gate=<grados>` - Controlar barrera manualmente
- `getPosition` - Obtener posición actual del servo
- `zero` - Cerrar barrera completamente

Este sistema integra el control de servomotor existente con la lógica completa de gestión de parqueo según los requerimientos especificados.