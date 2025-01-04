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
#include "utils/config.h"
#include "utils/logger.h"

#include <forward_list>
#include <notifications/notifications.h>
#include <wups.h>
#include <wups/button_combo/api.h>

// Mandatory plugin information.
WUPS_PLUGIN_NAME("SwipSwapMe");
WUPS_PLUGIN_DESCRIPTION("Swaps the GamePad and TV screen when pressing a certain button combination");
WUPS_PLUGIN_VERSION(PLUGIN_VERSION_FULL);
WUPS_PLUGIN_AUTHOR("Maschell");
WUPS_PLUGIN_LICENSE("GPL");

// FS Access
WUPS_USE_WUT_DEVOPTAB();

WUPS_USE_STORAGE("SwipSwapMeAroma");


namespace {
    uint32_t migrateButtonCombo(const uint32_t buttons) {
        uint32_t conv_buttons = 0;

        if (buttons & VPAD_BUTTON_A) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_A;
        }
        if (buttons & VPAD_BUTTON_B) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_B;
        }
        if (buttons & VPAD_BUTTON_X) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_X;
        }
        if (buttons & VPAD_BUTTON_Y) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_Y;
        }

        if (buttons & VPAD_BUTTON_LEFT) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_LEFT;
        }
        if (buttons & VPAD_BUTTON_RIGHT) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_RIGHT;
        }
        if (buttons & VPAD_BUTTON_UP) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_UP;
        }
        if (buttons & VPAD_BUTTON_DOWN) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_DOWN;
        }

        if (buttons & VPAD_BUTTON_ZL) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_ZL;
        }
        if (buttons & VPAD_BUTTON_ZR) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_ZR;
        }

        if (buttons & VPAD_BUTTON_L) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_L;
        }
        if (buttons & VPAD_BUTTON_R) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_R;
        }

        if (buttons & VPAD_BUTTON_PLUS) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_PLUS;
        }
        if (buttons & VPAD_BUTTON_MINUS) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_MINUS;
        }

        if (buttons & VPAD_BUTTON_STICK_R) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_STICK_R;
        }
        if (buttons & VPAD_BUTTON_STICK_L) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_STICK_L;
        }

        if (buttons & VPAD_BUTTON_TV) {
            conv_buttons |= WUPS_BUTTON_COMBO_BUTTON_TV;
        }

        return conv_buttons;
    }

    const char *screenModeToStr(const SwipSwapScreenMode mode) {
        switch (mode) {
            case SCREEN_MODE_NONE:
                return "Screen mode: Normal";
            case SCREEN_MODE_SWAP:
                return "Screen mode: Swapping TV and GamePad";
            case SCREEN_MODE_MIRROR_TV:
                return "Screen mode: Mirror TV to GamePad";
            case SCREEN_MODE_MIRROR_DRC:
                return "Screen mode: Mirror GamePad to TV";
            case SCREEN_MODE_MAX_VALUE:
                break;
        }
        return "Invalid screen mode";
    }


    const char *audioModeToStr(const SwipSwapAudioMode mode) {
        switch (mode) {
            case AUDIO_MODE_NONE:
                return "Audio mode: Normal";
            case AUDIO_MODE_SWAP:
                return "Audio mode: Swap TV and GamePad";
            case AUDIO_MODE_MATCH_SCREEN:
                return "Audio mode: Sound matches screen";
            case AUDIO_MODE_COMBINE:
                return "Audio mode: Combine TV and GamePad";
            case AUDIO_MODE_LEFT_TV_RIGHT_DRC:
                return "Audio mode: Left speaker TV, right speaker GamePad";
            case AUDIO_MODE_MAX_VALUE:
                break;
        }
        return "Invalid audio mode";
    }

    template<typename... Args>
    std::string string_format(const std::string &format, Args... args) {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
        auto size  = static_cast<size_t>(size_s);
        auto buf   = std::make_unique<char[]>(size);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

} // namespace


void SwapScreensCallback(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void *) {
    if (!gEnabled || !gSwapScreenButtonComboEnabled) { return; }
    if (gCurScreenMode == SCREEN_MODE_SWAP) {
        gCurScreenMode = SCREEN_MODE_NONE;
    } else {
        gCurScreenMode = SCREEN_MODE_SWAP;
    }

    if (gShowNotifications && gNotificationModuleInitDone) {
        NotificationModule_AddInfoNotification(screenModeToStr(gCurScreenMode));
    }
}

void ChangeScreensCallback(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void *) {
    if (!gEnabled || !gChangeScreenModeButtonComboEnabled) { return; }
    auto val = static_cast<uint32_t>(gCurScreenMode);
    val++;
    if (val >= SCREEN_MODE_MAX_VALUE) {
        val = 0;
    }
    gCurScreenMode = static_cast<SwipSwapScreenMode>(val);
    if (gShowNotifications && gNotificationModuleInitDone) {
        NotificationModule_AddInfoNotification(screenModeToStr(gCurScreenMode));
    }
}

void ChangeVoicesCallback(WUPSButtonCombo_ControllerTypes, WUPSButtonCombo_ComboHandle, void *) {
    if (!gEnabled || !gChangeAudioModeButtonComboEnabled) { return; }
    auto val = static_cast<uint32_t>(gCurAudioMode);
    val++;
    if (val >= AUDIO_MODE_MAX_VALUE) {
        val = 0;
    }
    gCurAudioMode = static_cast<SwipSwapAudioMode>(val);
    if (gShowNotifications && gNotificationModuleInitDone) {
        NotificationModule_AddInfoNotification(audioModeToStr(gCurAudioMode));
    }
}

void migrateStorage() {
    uint32_t doSwap = false;
    if (WUPSStorageAPI::Get(SWAP_SCREENS_CONFIG_STRING_DEPRECATED, doSwap) == WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_INFO("Found deprecated config in storage. Storage will be migrated");
        if (doSwap) {
            gCurScreenMode = SCREEN_MODE_SWAP;
        }
        if (WUPSStorageAPI::DeleteItem(SWAP_SCREENS_CONFIG_STRING_DEPRECATED) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to delete deprecated value: \"%s\" from storage", SWAP_SCREENS_CONFIG_STRING_DEPRECATED);
        }
    }

    uint32_t oldButtonCombo = 0;
    if (WUPSStorageAPI::Get(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED, oldButtonCombo) == WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_INFO("Found deprecated config in storage. Storage will be migrated");
        gSwapScreenButtonCombo = migrateButtonCombo(oldButtonCombo);
        if (WUPSStorageAPI::DeleteItem(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to delete deprecated value: \"%s\" from storage", SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED);
        }
    }
    if (WUPSStorageAPI::Get(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING_DEPRECATED, oldButtonCombo) == WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_INFO("Found deprecated config in storage. Storage will be migrated");
        gSwapAudioButtonCombo = migrateButtonCombo(oldButtonCombo);
        if (WUPSStorageAPI::DeleteItem(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING_DEPRECATED) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to delete deprecated value: \"%s\" from storage", CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING_DEPRECATED);
        }
    }
    if (WUPSStorageAPI::Get(CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED, oldButtonCombo) == WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_INFO("Found deprecated config in storage. Storage will be migrated");
        gChangeScreenButtonCombo = migrateButtonCombo(oldButtonCombo);
        if (WUPSStorageAPI::DeleteItem(CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED) != WUPS_STORAGE_ERROR_SUCCESS) {
            DEBUG_FUNCTION_LINE_WARN("Failed to delete deprecated value: \"%s\" from storage", CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED);
        }
    }
}

void showNotificationOnFirstBoot() {
    uint32_t val;
    if (WUPSStorageAPI::Get<uint32_t>(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, val) == WUPS_STORAGE_ERROR_NOT_FOUND) {
        NotificationModule_AddInfoNotification("Press \ue089+\ue07C to swap the TV and Gamepad screen! You can change this button combination at any time in the config menu");
    }
}

std::forward_list<WUPSButtonComboAPI::ButtonCombo> sButtonCombos;

WUPSButtonCombo_ComboHandle RegisterButtonCombo(const std::string_view label, const WUPSButtonCombo_Buttons buttonCombo, const WUPSButtonCombo_ComboCallback callback) {
    const auto buttonComboLabel        = string_format("SwipSwapMe: %s", label.data());
    WUPSButtonCombo_ComboStatus status = WUPS_BUTTON_COMBO_COMBO_STATUS_INVALID_STATUS;
    WUPSButtonCombo_Error err          = WUPS_BUTTON_COMBO_ERROR_UNKNOWN_ERROR;
    auto res                           = WUPSButtonComboAPI::CreateComboPressDown(buttonComboLabel,
                                                                                  buttonCombo,
                                                                                  callback,
                                                                                  nullptr,
                                                                                  status, err);
    if (!res || err != WUPS_BUTTON_COMBO_ERROR_SUCCESS) {
        const std::string errorMsg = string_format("SwipSwapMe: Failed to register button combo \"%s\"", label.data());
        DEBUG_FUNCTION_LINE_ERR("%s", errorMsg.c_str());
        NotificationModule_AddErrorNotification(errorMsg.c_str());
    } else {
        if (status == WUPS_BUTTON_COMBO_COMBO_STATUS_CONFLICT) {
            const auto conflictMsg = string_format("SwipSwapMe: \"%s\"-combo was disabled due to a conflict. Please assign a different combo", label.data());
            DEBUG_FUNCTION_LINE_INFO("%s", conflictMsg.c_str());
            NotificationModule_AddInfoNotification(conflictMsg.c_str());
        } else if (status != WUPS_BUTTON_COMBO_COMBO_STATUS_VALID) {
            const auto conflictMsg = string_format("SwipSwapMe: Unknown error happened while registering button combo \"%s\"", label.data());
            DEBUG_FUNCTION_LINE_INFO("%s", conflictMsg.c_str());
            NotificationModule_AddInfoNotification(conflictMsg.c_str());
        }
        const auto handle = res->getHandle();
        sButtonCombos.emplace_front(std::move(*res));
        return handle;
    }
    return WUPSButtonCombo_ComboHandle(nullptr);
}

// Gets called once the loader exists.
INITIALIZE_PLUGIN() {
    initLogging();

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, true);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 20.0f);

    if (NotificationModule_InitLibrary() == NOTIFICATION_MODULE_RESULT_SUCCESS) {
        gNotificationModuleInitDone = true;
    } else {
        gNotificationModuleInitDone = false;
        DEBUG_FUNCTION_LINE_ERR("Failed to init notification lib");
    }

    gCurScreenMode           = DEFAULT_SCREEN_MODE_CONFIG_VALUE;                // migrateStorage might override this
    gSwapScreenButtonCombo   = DEFAULT_SWAP_SCREEN_BUTTON_COMBO_CONFIG_VALUE;   // migrateStorage might override this
    gSwapAudioButtonCombo    = DEFAULT_CHANGE_AUDIO_BUTTON_COMBO_CONFIG_VALUE;  // migrateStorage might override this
    gChangeScreenButtonCombo = DEFAULT_CHANGE_SCREEN_BUTTON_COMBO_CONFIG_VALUE; // migrateStorage might override this
    migrateStorage();
    showNotificationOnFirstBoot();

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
    if ((err = WUPSStorageAPI::GetOrStoreDefault(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, gSwapScreenButtonCombo, gSwapScreenButtonCombo)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING, gSwapAudioButtonCombo, gSwapAudioButtonCombo)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING, gChangeScreenButtonCombo, gChangeScreenButtonCombo)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }
    if ((err = WUPSStorageAPI::GetOrStoreDefault(AUDIO_MODE_CONFIG_STRING, gCurAudioMode, DEFAULT_AUDIO_MODE_CONFIG_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to get or create item \"%s\": %s (%d)", AUDIO_MODE_CONFIG_STRING, WUPSStorageAPI_GetStatusStr(err), err);
    }

    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to save storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }

    gSwapScreenButtonComboHandle   = RegisterButtonCombo("Swap Screens", static_cast<WUPSButtonCombo_Buttons>(gSwapScreenButtonCombo), SwapScreensCallback);
    gSwapAudioButtonComboHandle    = RegisterButtonCombo("Change Audio Mode", static_cast<WUPSButtonCombo_Buttons>(gSwapAudioButtonCombo), ChangeVoicesCallback);
    gChangeScreenButtonComboHandle = RegisterButtonCombo("Change Screen Mode", static_cast<WUPSButtonCombo_Buttons>(gChangeScreenButtonCombo), ChangeScreensCallback);

    WUPSConfigAPIOptionsV1 configOptions = {.name = "Swip Swap Me"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback) != WUPSCONFIG_API_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init config api");
    }

    deinitLogging();

    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 2.0f);
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_KEEP_UNTIL_SHOWN, false);
}

DEINITIALIZE_PLUGIN() {
    if (gNotificationModuleInitDone) {
        NotificationModule_DeInitLibrary();
        gNotificationModuleInitDone = false;
    }
    sButtonCombos.clear();
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
