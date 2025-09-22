#include "IPlatform.hpp"

#ifdef ESP32_PLATFORM
#include "ArduinoPlatform.hpp"
static ArduinoPlatform platformInstance;
#elif defined(NATIVE_PLATFORM)
#include "NativePlatform.hpp"
static NativePlatform platformInstance;
#endif

// Global platform instance
IPlatform* g_platform = &platformInstance;