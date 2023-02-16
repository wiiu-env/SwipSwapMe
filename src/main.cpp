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
#include <string>
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

// Gets called once the loader exists.
INITIALIZE_PLUGIN() {
    initLogging();

    if (NotificationModule_InitLibrary() == NOTIFICATION_MODULE_RESULT_SUCCESS) {
        gNotificationModuleInitDone = true;
    } else {
        gNotificationModuleInitDone = false;
        DEBUG_FUNCTION_LINE_ERR("Failed to init notification lib");
    }

    // Open storage to read values
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
    } else {
        LOAD_BOOL_FROM_STORAGE(ENABLED_CONFIG_STRING, gEnabled);
        LOAD_BOOL_FROM_STORAGE(SWAP_SCREENS_CONFIG_STRING, gDoScreenSwap);
        LOAD_BOOL_FROM_STORAGE(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING, gSwapScreenButtonComboEnabled);
        LOAD_BOOL_FROM_STORAGE(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING, gChangeAudioModeButtonComboEnabled);
        LOAD_BOOL_FROM_STORAGE(ENABLE_NOTIFICATIONS_CONFIG_STRING, gShowNotifications);
        LOAD_INT_FROM_STORAGE(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, gSwapScreenButtonCombo);
        LOAD_INT_FROM_STORAGE(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING, gSwapAudioButtonCombo);
        LOAD_INT_FROM_STORAGE(SCREEN_MODE_CONFIG_STRING, gCurAudioMode);

        // Close storage
        if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
        }
    }
    deinitLogging();
}

// Called whenever an application was started.
ON_APPLICATION_START() {
    initLogging();
}

ON_APPLICATION_REQUESTS_EXIT() {
    // Open storage to write current config
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage %s (%d)", WUPS_GetStorageStatusStr(storageRes), storageRes);
    } else {
        WUPS_StoreInt(nullptr, SCREEN_MODE_CONFIG_STRING, (int32_t) gCurAudioMode);
        WUPS_StoreBool(nullptr, SWAP_SCREENS_CONFIG_STRING, (int32_t) gDoScreenSwap);

        // Close storage
        if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
        }
    }
    deinitLogging();
}

ON_APPLICATION_ENDS() {
    if (gNotificationModuleInitDone) {
        NotificationModule_DeInitLibrary();
        gNotificationModuleInitDone = false;
    }
}