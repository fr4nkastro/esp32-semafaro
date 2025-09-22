#pragma once
#include <Arduino.h>

// Sistema simple de logging con niveles
#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERR(msg, ...) do { Serial.printf("[ERROR] " msg "\n", ##__VA_ARGS__); } while(0)
#else
#define LOG_ERR(msg, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(msg, ...) do { Serial.printf("[WARN ] " msg "\n", ##__VA_ARGS__); } while(0)
#else
#define LOG_WARN(msg, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(msg, ...) do { Serial.printf("[INFO ] " msg "\n", ##__VA_ARGS__); } while(0)
#else
#define LOG_INFO(msg, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(msg, ...) do { Serial.printf("[DEBUG] " msg "\n", ##__VA_ARGS__); } while(0)
#else
#define LOG_DEBUG(msg, ...)
#endif