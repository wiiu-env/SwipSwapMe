/****************************************************************************
 * Copyright (C) 2017,2018 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "main.h"
#include "retain_vars.hpp"
#include "utils/WUPSConfigItemButtonCombo.h"
#include "utils/config.h"
#include "utils/logger.h"
#include <notifications/notifications.h>
#include <wups.h>

// Mandatory plugin information.
WUPS_PLUGIN_NAME("SwipSwapMe");
WUPS_PLUGIN_DESCRIPTION("Swaps the GamePad and TV screen when pressing a certain button combination");
WUPS_PLUGIN_VERSION(PLUGIN_VERSION_FULL);
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

// FS Access
WUPS_USE_WUT_DEVOPTAB();

WUPS_USE_STORAGE("SwipSwapMeAroma");

void migrateStorage() {
    uint32_t doSwap = false;
    if (WUPSStorageAPI_GetU32(nullptr, SWAP_SCREENS_CONFIG_STRING_DEPRECATED, &doSwap) == WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_INFO("Found deprecated config in storage. Storage will be migrated");
        if (doSwap) {
            gCurScreenMode = SCREEN_MODE_SWAP;
        }
        if (WUPSStorageAPI_DeleteItem(nullptr, SWAP_SCREENS_CONFIG_STRING_DEPRECATED) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to delete deprecated value: \"%s\" from storage", SWAP_SCREENS_CONFIG_STRING_DEPRECATED);
        }
    }
}

// Gets called once the loader exists.
INITIALIZE_PLUGIN() {
    initLogging();

    if (NotificationModule_InitLibrary() == NOTIFICATION_MODULE_RESULT_SUCCESS) {
        gNotificationModuleInitDone = true;
    } else {
        gNotificationModuleInitDone = false;
        DEBUG_FUNCTION_LINE_ERR("Failed to init notification lib");
    }

    gCurScreenMode = DEFAULT_SCREEN_MODE_CONFIG_VALUE; // migrateStorage might override this
    migrateStorage();

    WUPSStorageError err;
    if ((err = WUPSStorageAPI::GetOrStoreDefault(ENABLED_CONFIG_STRING, gEnabled, DEFAULT_ENABLED_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", ENABLED_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(SCREEN_MODE_CONFIG_STRING, gCurScreenMode, gCurScreenMode)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", SCREEN_MODE_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING, gSwapScreenButtonComboEnabled, DEFAULT_ENABLED_SWAP_SCREENS_COMBO_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING, gChangeAudioModeButtonComboEnabled, DEFAULT_ENABLED_CHANGE_AUDIO_COMBO_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(ENABLED_CHANGE_SCREEN_COMBO_CONFIG_STRING, gChangeScreenModeButtonComboEnabled, DEFAULT_ENABLED_CHANGE_SCREEN_COMBO_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", ENABLED_CHANGE_SCREEN_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(ENABLE_NOTIFICATIONS_CONFIG_STRING, gShowNotifications, DEFAULT_ENABLE_NOTIFICATIONS_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", ENABLE_NOTIFICATIONS_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, gSwapScreenButtonCombo, (uint32_t) DEFAULT_SWAP_SCREEN_BUTTON_COMBO_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING, gSwapAudioButtonCombo, (uint32_t) DEFAULT_CHANGE_AUDIO_BUTTON_COMBO_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING, gChangeScreenButtonCombo, (uint32_t) DEFAULT_CHANGE_SCREEN_BUTTON_COMBO_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(AUDIO_MODE_CONFIG_STRING, gCurAudioMode, DEFAULT_AUDIO_MODE_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", AUDIO_MODE_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }

    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }

    WUPSConfigAPIOptionsV1 configOptions = {.name = "Swip Swap Me"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
    }

    deinitLogging();
}

DEINITIALIZE_PLUGIN() {
    if (gNotificationModuleInitDone) {
        NotificationModule_DeInitLibrary();
        gNotificationModuleInitDone = false;
    }
}

static SwipSwapAudioMode sAudioModeAtStart;
static SwipSwapScreenMode sScreenModeAtStart;

// Called whenever an application was started.
ON_APPLICATION_START() {
    initLogging();

    sAudioModeAtStart  = gCurAudioMode;
    sScreenModeAtStart = gCurScreenMode;
}

ON_APPLICATION_REQUESTS_EXIT() {
    if (sAudioModeAtStart != gCurAudioMode || sScreenModeAtStart != gCurScreenMode) {
        WUPSStorageError err;
        if ((err = WUPSStorageAPI::Store(AUDIO_MODE_CONFIG_STRING, gCurAudioMode)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to store audio mode to storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
        }

        if ((err = WUPSStorageAPI::Store(SCREEN_MODE_CONFIG_STRING, gCurScreenMode)) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to store screen mode to storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
        }

        if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to save storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
        }
    }

    deinitLogging();
}
