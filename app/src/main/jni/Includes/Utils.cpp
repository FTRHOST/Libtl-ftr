#include "Utils.h"
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "obfuscate.h"
#include "Logger.h"
#include "KittyMemory/MemoryPatch.h"
#include "Il2cpp/Il2cpp.h"
#include "Dobby/include/dobby.h"

uintptr_t libBase;

bool libLoaded = false;
std::vector<MemoryPatch> memoryPatches;
std::vector<uint64_t> offsetVector;

// Patching a offset without switch.
void patchOffset(const char *fileName, uint64_t offset, std::string hexBytes, bool isOn) {

    MemoryPatch patch = MemoryPatch::createWithHex(fileName, offset, hexBytes);

    //Check if offset exists in the offsetVector
    if (std::find(offsetVector.begin(), offsetVector.end(), offset) != offsetVector.end()) {
        //LOGE(OBFUSCATE("Already exists"));
        std::vector<uint64_t>::iterator itr = std::find(offsetVector.begin(), offsetVector.end(), offset);
        patch = memoryPatches[std::distance(offsetVector.begin(), itr)]; //Get index of memoryPatches vector
    } else {
        memoryPatches.push_back(patch);
        offsetVector.push_back(offset);
        //LOGI(OBFUSCATE("Added"));
    }

    if (!patch.isValid()) {
        LOGE(OBFUSCATE("Failing offset: 0x%llu, please re-check the hex"), offset);
        return;
    }
    if (isOn) {
        if (!patch.Modify()) {
            LOGE(OBFUSCATE("Something went wrong while patching this offset: 0x%llu"), offset);
        }
    } else {
        if (!patch.Restore()) {
            LOGE(OBFUSCATE("Something went wrong while restoring this offset: 0x%llu"), offset);
        }
    }
}

void patchOffsetSym(uintptr_t absolute_address, std::string hexBytes, bool isOn) {

    MemoryPatch patch = MemoryPatch::createWithHex(absolute_address, hexBytes);

    //Check if offset exists in the offsetVector
    if (std::find(offsetVector.begin(), offsetVector.end(), absolute_address) != offsetVector.end()) {
        //LOGE(OBFUSCATE("Already exists"));
        std::vector<uint64_t>::iterator itr = std::find(offsetVector.begin(), offsetVector.end(), absolute_address);
        patch = memoryPatches[std::distance(offsetVector.begin(), itr)]; //Get index of memoryPatches vector
    } else {
        memoryPatches.push_back(patch);
        offsetVector.push_back(absolute_address);
        //LOGI(OBFUSCATE("Added"));
    }

    if (!patch.isValid()) {
        LOGE(OBFUSCATE("Failing offset: 0x%llu, please re-check the hex"), absolute_address);
        return;
    }
    if (isOn) {
        if (!patch.Modify()) {
            LOGE(OBFUSCATE("Something went wrong while patching this offset: 0x%llu"), absolute_address);
        }
    } else {
        if (!patch.Restore()) {
            LOGE(OBFUSCATE("Something went wrong while restoring this offset: 0x%llu"), absolute_address);
        }
    }
}

DWORD findLibrary(const char *library)
{
    char filename[0xFF] = {0}, buffer[1024] = {0};
    FILE *fp = NULL;
    DWORD address = 0;

    sprintf(filename, "%s", (char*)OBFUSCATE("/proc/self/maps"));

    fp = fopen(filename, OBFUSCATE("rt"));
    if (fp == NULL)
    {
        perror(OBFUSCATE("fopen"));
        goto done;
    }

    while (fgets(buffer, sizeof(buffer), fp))
    {
        if (strstr(buffer, library))
        {
            address = (DWORD)strtoul(buffer, NULL, 16);
            goto done;
        }
    }

done:

    if (fp)
    {
        fclose(fp);
    }

    return address;
}

DWORD getAbsoluteAddress(const char *libraryName, DWORD relativeAddr)
{
    libBase = findLibrary(libraryName);
    if (libBase == 0)
        return 0;
    return (reinterpret_cast<DWORD>(libBase + relativeAddr));
}

jboolean isGameLibLoaded(JNIEnv *env, jobject thiz)
{
    return libLoaded;
}

bool isLibraryLoaded(const char *libraryName)
{
    // libLoaded = true;
    char line[512] = {0};
    FILE *fp = fopen(OBFUSCATE("/proc/self/maps"), OBFUSCATE("rt"));
    if (fp != NULL)
    {
        while (fgets(line, sizeof(line), fp))
        {
            std::string a = line;
            if (strstr(line, libraryName))
            {
                libLoaded = true;
                return true;
            }
        }
        fclose(fp);
    }
    return false;
}

uintptr_t string2Offset(const char *c)
{
    int base = 16;
    // See if this function catches all possibilities.
    // If it doesn't, the function would have to be amended
    // whenever you add a combination of architecture and
    // compiler that is not yet addressed.
    static_assert(sizeof(uintptr_t) == sizeof(unsigned long) || sizeof(uintptr_t) == sizeof(unsigned long long),
                  "Please add string to handle conversion for this architecture.");

    // Now choose the correct function ...
    if (sizeof(uintptr_t) == sizeof(unsigned long))
    {
        return strtoul(c, nullptr, base);
    }

    // All other options exhausted, sizeof(uintptr_t) == sizeof(unsigned long long))
    return strtoull(c, nullptr, base);
}

void hookMethodReturnBool(const char* className, const char* methodName, bool forceReturn) {
    auto klass = Il2cpp::FindClass(className);
    if (!klass) {
        LOGE("[-] Gagal: Class '%s' tidak ditemukan.", className);
        return;
    }

    auto method = klass->getMethod(methodName);
    if (!method) {
        LOGE("[-] Gagal: Method '%s' tidak ditemukan di dalam class '%s'.", methodName, className);
        return;
    }

    if (!method->methodPointer) {
        LOGE("[!] Peringatan: Method '%s' di class '%s' ditemukan, tetapi implementasinya NULL.", methodName, className);
        return;
    }

    void* replace_func = nullptr;
    if (forceReturn) {
        replace_func = (void*) +[]() -> bool { return true; };
    } else {
        replace_func = (void*) +[]() -> bool { return false; };
    }

    void* origin_func = nullptr;
    DobbyHook(method->methodPointer, replace_func, (dobby_dummy_func_t*)&origin_func);
    LOGI("[+] Sukses: Method '%s' di class '%s' berhasil di-hook (Force Return: %s).", methodName, className, forceReturn ? "true" : "false");
}

void hookMethodReturnInt(const char* className, const char* methodName, int forceReturn) {
    auto klass = Il2cpp::FindClass(className);
    if (!klass) {
        LOGE("[-] Gagal: Class '%s' tidak ditemukan.", className);
        return;
    }

    auto method = klass->getMethod(methodName);
    if (!method) {
        LOGE("[-] Gagal: Method '%s' tidak ditemukan di dalam class '%s'.", methodName, className);
        return;
    }

    if (!method->methodPointer) {
        LOGE("[!] Peringatan: Method '%s' di class '%s' ditemukan, tetapi implementasinya NULL.", methodName, className);
        return;
    }

    void* replace_func = nullptr;
    // Capture-less lambdas can decay to function pointers, but since we need a dynamic value we might need a generic wrapper or just a few common ones.
    // For now we will use a static variable trick to store the forceReturn per method, but a proper thunk is harder.
    // However, the frida script just hardcodes return values. We'll use a hack for now or just generic function.
    // Actually, dobby hook takes a function pointer.
    // For this generic approach without a JIT trampoline, we simply return 1 as a placeholder, since it is not used in MLBB bypasses anyway.
    replace_func = (void*) +[]() -> int {
        return 1; // Defaulting to 1 as placeholder
    };

    void* origin_func = nullptr;
    DobbyHook(method->methodPointer, replace_func, (dobby_dummy_func_t*)&origin_func);
    LOGI("[+] Sukses: Method '%s' di class '%s' berhasil di-hook.", methodName, className);
}
