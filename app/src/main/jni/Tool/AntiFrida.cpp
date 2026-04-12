#include "AntiFrida.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>

#include "../Includes/Logger.h" // Optional, for logging

namespace AntiFrida
{
    std::atomic<bool> is_frida_detected{false};

    // Fungsi untuk menscan /proc/net/tcp untuk mencari port 27042 (69A2)
    bool ScanPort27042()
    {
        std::ifstream tcp_file("/proc/net/tcp");
        if (!tcp_file.is_open())
        {
            return false;
        }

        std::string line;
        // Skip header
        std::getline(tcp_file, line);

        while (std::getline(tcp_file, line))
        {
            std::istringstream iss(line);
            std::string sl, local_address, rem_address, st;

            // Format /proc/net/tcp:
            //  sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
            if (iss >> sl >> local_address >> rem_address >> st)
            {
                // Ekstrak port dari local_address (format: IP:PORT dalam Hex)
                size_t colon_pos = local_address.find(':');
                if (colon_pos != std::string::npos)
                {
                    std::string port_hex = local_address.substr(colon_pos + 1);
                    // 27042 in hex is 69A2 (or 69a2)
                    if (port_hex == "69A2" || port_hex == "69a2")
                    {
                        // Cek status "0A" (Listening)
                        if (st == "0A")
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    // Fungsi untuk menscan /proc/self/maps untuk frida-agent
    bool ScanMemoryMaps()
    {
        std::ifstream maps_file("/proc/self/maps");
        if (!maps_file.is_open())
        {
            return false;
        }

        std::string line;
        while (std::getline(maps_file, line))
        {
            // Abaikan file yang berkaitan dengan "frida-gum" karena internal library
            if (line.find("frida-gum") != std::string::npos)
            {
                continue;
            }

            // Cari string "frida-agent" atau "frida-agent-64"
            if (line.find("frida-agent") != std::string::npos || line.find("frida-agent-64") != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    // Fungsi loop monitor di background thread
    void MonitorLoop()
    {
        while (true)
        {
            if (ScanPort27042() || ScanMemoryMaps())
            {
                is_frida_detected = true;
                LOGE("[!] Frida Server/Client Detected!");
            }

            // Tunggu 3 detik sebelum cek lagi
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    // Memulai thread monitor
    void StartMonitor()
    {
        std::thread monitor_thread(MonitorLoop);
        monitor_thread.detach(); // Biarkan berjalan di background
    }
}
