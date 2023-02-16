#pragma once
#include <coreinit/dynload.h>

inline bool isModuleLoaded(const char *name) {
    OSDynLoad_Module module;
    auto err = OSDynLoad_IsModuleLoaded(name, &module);

    if (err != OS_DYNLOAD_OK || !module) {
        return false;
    }
    return true;
}