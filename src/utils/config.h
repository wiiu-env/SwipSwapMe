#pragma once
#include "logger.h"
#include <string>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/storage.h>

#define ENABLED_CONFIG_STRING                    "enabled"
#define SWAP_SCREENS_CONFIG_STRING               "swapScreens"
#define ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING "swapScreensComboEnabled"
#define ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING "changeAudioComboEnabled"
#define ENABLE_NOTIFICATIONS_CONFIG_STRING       "notificationsEnabled"
#define SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING   "screenButtonCombo"
#define CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING  "audioButtonCombo"
#define SCREEN_MODE_CONFIG_STRING                "audioMode"

#define LOAD_BOOL_FROM_STORAGE(config_name, __variable__)                                                         \
    if ((storageRes = WUPS_GetBool(nullptr, config_name, &__variable__)) == WUPS_STORAGE_ERROR_NOT_FOUND) {       \
        if (WUPS_StoreBool(nullptr, config_name, __variable__) != WUPS_STORAGE_ERROR_SUCCESS) {                   \
            DEBUG_FUNCTION_LINE_WARN("Failed to store bool");                                                     \
        }                                                                                                         \
    } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {                                                        \
        DEBUG_FUNCTION_LINE_WARN("Failed to get bool %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes); \
    }                                                                                                             \
    while (0)

#define LOAD_STRING_FROM_STORAGE(config_name, __string, __string_length)                                                  \
    if ((storageRes = WUPS_GetString(nullptr, config_name, __string, __string_length)) == WUPS_STORAGE_ERROR_NOT_FOUND) { \
        if (WUPS_StoreString(nullptr, config_name, __string) != WUPS_STORAGE_ERROR_SUCCESS) {                             \
            DEBUG_FUNCTION_LINE_WARN("Failed to store string");                                                           \
        }                                                                                                                 \
    } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {                                                                \
        DEBUG_FUNCTION_LINE_WARN("Failed to get bool %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);         \
    }                                                                                                                     \
    while (0)

#define LOAD_INT_FROM_STORAGE(config_name, __intValue)                                                               \
    if ((storageRes = WUPS_GetInt(nullptr, config_name, (int32_t *) &__intValue)) == WUPS_STORAGE_ERROR_NOT_FOUND) { \
        if (WUPS_StoreInt(nullptr, config_name, (int32_t) __intValue) != WUPS_STORAGE_ERROR_SUCCESS) {               \
            DEBUG_FUNCTION_LINE_WARN("Failed to store int");                                                         \
        }                                                                                                            \
    } else if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {                                                           \
        DEBUG_FUNCTION_LINE_WARN("Failed to get int %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);     \
    }                                                                                                                \
    while (0)

#define PROCESS_BOOL_ITEM_CHANGED(__config__name, __variable__)                   \
    if (std::string_view(item->configId) == __config__name) {                     \
        DEBUG_FUNCTION_LINE_ERR("New value in %s: %d", __config__name, newValue); \
        __variable__ = newValue;                                                  \
        WUPS_StoreInt(nullptr, __config__name, __variable__);                     \
        return;                                                                   \
    }                                                                             \
    while (0)
