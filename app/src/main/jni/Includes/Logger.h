#pragma once
#include <jni.h>
#include <android/log.h>

// pasted straight from the imgui_demo.cpp
namespace logger
{
    void Clear();

    void AddLog(const char *prefix, const char *fmt, ...);

    void DebugLog(const char *fmt, ...);

    void Draw(const char *title, bool *p_open = 0);
}; // namespace logger

#define __LOG_ALL__

#define LOG_TAG "ftr"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define LOGPTR(ptr) LOGD(#ptr " => %p", ptr)
#define LOGHEX(ptr) LOGD(#ptr " => 0x%llX", ptr)
#define LOGINT(var) LOGD(#var " => %d", var)
#define LOGSINGLE(var) LOGD(#var " => %f", var)
#define LOGSTR(il2cppstring) LOGD(#il2cppstring " => %s", il2cppstring->to_string().c_str())
