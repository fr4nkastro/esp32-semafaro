#pragma once
#include "platform/IPlatform.hpp"
#include <functional>
#include <vector>

class Scheduler {
public:
  using Task = std::function<void()>;
  
  // Programar una tarea para ejecutar cada 'intervalMs' milisegundos
  static void every(uint32_t intervalMs, Task task);
  
  // Ejecutar todas las tareas pendientes (llamar en loop())
  static void tick();
  
  // Limpiar todas las tareas programadas
  static void clear();

private:
  struct ScheduledTask {
    uint32_t intervalMs;
    uint32_t lastRunMs;
    Task task;
  };
  
  static std::vector<ScheduledTask> tasks_;
};