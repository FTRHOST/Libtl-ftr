#pragma once
#include "Il2cpp/il2cpp-class.h"
#include "Tool/Tool.h"

#include <mutex>
#include <string>
#include <vector>

namespace Frida
{
    struct TraceData
    {
        MethodInfo *method = nullptr;
        std::string msg;
    };
    void Init();
    bool Trace(MethodInfo *method, HookerData *data);
    bool Untrace(MethodInfo *method);
    bool isTraced(MethodInfo *method);

    namespace ScriptManager
    {
        extern std::vector<std::string> frida_logs;
        extern std::mutex log_mutex;

        void InitGum();
        bool LoadScriptFromFile(const std::string& filepath);
        bool LoadScriptFromRawCode(const std::string& source);
        void ClearLogs();
    }
} // namespace Frida
