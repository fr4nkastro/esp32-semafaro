#include "Scheduler.hpp"

// Definición del vector estático
std::vector<Scheduler::ScheduledTask> Scheduler::tasks_;

void Scheduler::every(uint32_t intervalMs, Task task) {
  tasks_.push_back({
    .intervalMs = intervalMs,
    .lastRunMs = millis(),
    .task = task
  });
}

void Scheduler::tick() {
  uint32_t now = millis();
  
  for (auto& scheduledTask : tasks_) {
    if (now - scheduledTask.lastRunMs >= scheduledTask.intervalMs) {
      scheduledTask.task();
      scheduledTask.lastRunMs = now;
    }
  }
}

void Scheduler::clear() {
  tasks_.clear();
}