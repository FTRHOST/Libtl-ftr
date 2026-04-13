#pragma once
#include "Il2cpp/il2cpp-class.h"
#include "Includes/circular_buffer.h"
#include "Tool/ClassesTab.h"
#include <set>

struct HookerTrace
{
    std::string name;
    float time;
    float goneTime;
    int hitCount;
};

struct HookerData
{
    int hitCount = 0;
    float time = 0.f;
    MethodInfo *method = nullptr;
    bool backtracing = false;
    CircularBuffer<std::vector<std::string>> backtraced{10};
    static CircularBuffer<HookerTrace> visited;
    static std::unordered_map<Il2CppClass *, std::set<Il2CppObject *>> collectSet;
};
struct SavedPatch
{
    std::string imageName;
    std::string className;
    std::string methodName;
    size_t argsCount;
    std::string patchText;
};

void to_json(nlohmann::ordered_json &j, const SavedPatch &p);
void from_json(const nlohmann::ordered_json &j, SavedPatch &p);

namespace Tool
{
    extern std::vector<SavedPatch> savedPatches;
    void ConfigSave();
    void ConfigLoad();
    void LoadPatches();
    void SavePatches();
    void AddSavedPatch(MethodInfo* method, const std::string& patchText);
    void RemoveSavedPatch(MethodInfo* method);
    void ApplySavedPatches();
    void Init(Il2CppImage *image, std::vector<Il2CppImage *> images);
    void FilterClasses(const std::string &filter);
    void Draw();
    void Tracer();
    void Hooker();
    void GameObjects();
    void Dumper();
    bool ToggleHooker(MethodInfo *method, int state = -1);
    void CalculateSomething();
    ClassesTab &GetFirstTab(); // TODO: maybe just return classesTabs
    ClassesTab &OpenNewTab();
    ClassesTab &OpenNewTabFromClass(Il2CppClass *klass);
} // namespace Tool
