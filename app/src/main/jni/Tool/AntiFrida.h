#pragma once

#include <atomic>
#include <string>

namespace AntiFrida
{
    // Global variable (thread-safe) to indicate if Frida is detected
    extern std::atomic<bool> is_frida_detected;

    // Detection methods
    bool ScanPort27042();
    bool ScanMemoryMaps();

    // Lifecycle management
    void StartMonitor();
}
