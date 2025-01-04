#include "config.h"

#include "logger.h"
#include "retain_vars.hpp"

#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemButtonCombo.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/storage.h>

extern void UpdateAudioMode();

static void boolItemChangedConfig(ConfigItemBoolean *item, bool newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in bool item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);
    if (std::string_view(ENABLED_CONFIG_STRING) == item->identifier) {
        gEnabled = newValue;
        UpdateAudioMode();
    } else if (std::string_view(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING) == item->identifier) {
        gSwapScreenButtonComboEnabled = newValue;
    } else if (std::string_view(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING) == item->identifier) {
        gChangeAudioModeButtonComboEnabled = newValue;
    } else if (std::string_view(ENABLED_CHANGE_SCREEN_COMBO_CONFIG_STRING) == item->identifier) {
        gChangeScreenModeButtonComboEnabled = newValue;
    } else if (std::string_view(ENABLE_NOTIFICATIONS_CONFIG_STRING) == item->identifier) {
        gShowNotifications = newValue;
    } else {
        DEBUG_FUNCTION_LINE_WARN("Unexpected boolean item: %s", item->identifier);
        return;
    }
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::Store(item->identifier, newValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", newValue, item->identifier, WUPSStorageAPI_GetStatusStr(err), err);
    }
}

static void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in button combo item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);
    if (std::string_view(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING) == item->identifier) {
        gSwapScreenButtonCombo = newValue;
    } else if (std::string_view(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING) == item->identifier) {
        gSwapAudioButtonCombo = newValue;
    } else if (std::string_view(CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING) == item->identifier) {
        gChangeScreenButtonCombo = newValue;
    } else {
        DEBUG_FUNCTION_LINE_WARN("Unexpected button combo item: %s", item->identifier);
        return;
    }

    WUPSStorageError err;
    if ((err = WUPSStorageAPI::Store(item->identifier, newValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", newValue, item->identifier, WUPSStorageAPI_GetStatusStr(err), err);
    }
}

static void multiItemChanged(ConfigItemMultipleValues *item, uint32_t newValue) {
    if (!item || !item->identifier) {
        DEBUG_FUNCTION_LINE_WARN("Invalid item or identifier in multi item callback");
        return;
    }
    DEBUG_FUNCTION_LINE_VERBOSE("New value in %s changed: %d", item->identifier, newValue);

    if (std::string_view(AUDIO_MODE_CONFIG_STRING) == item->identifier) {
        gCurAudioMode = static_cast<SwipSwapAudioMode>(newValue);
    } else if (std::string_view(SCREEN_MODE_CONFIG_STRING) == item->identifier) {
        gCurScreenMode = static_cast<SwipSwapScreenMode>(newValue);
    } else {
        DEBUG_FUNCTION_LINE_WARN("Unexpected button combo item: %s", item->identifier);
        return;
    }

    WUPSStorageError err;
    if ((err = WUPSStorageAPI::Store(item->identifier, newValue)) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_WARN("Failed to store value %d to storage item \"%s\": %s (%d)", newValue, item->identifier, WUPSStorageAPI_GetStatusStr(err), err);
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        root.add(WUPSConfigItemBoolean::Create(ENABLED_CONFIG_STRING,
                                               "Plugin enabled",
                                               DEFAULT_ENABLED_CONFIG_VALUE, gEnabled,
                                               &boolItemChangedConfig));
        root.add(WUPSConfigItemBoolean::Create(ENABLE_NOTIFICATIONS_CONFIG_STRING,
                                               "Show notifications",
                                               DEFAULT_ENABLE_NOTIFICATIONS_CONFIG_VALUE, gShowNotifications,
                                               &boolItemChangedConfig));

        constexpr WUPSConfigItemMultipleValues::ValuePair screenModeMap[] = {
                {SCREEN_MODE_NONE, "Normal"},
                {SCREEN_MODE_SWAP, "Swap TV and GamePad"},
                {SCREEN_MODE_MIRROR_TV, "Mirror TV"},
                {SCREEN_MODE_MIRROR_DRC, "Mirror GamePad"},
        };

        root.add(WUPSConfigItemMultipleValues::CreateFromValue(SCREEN_MODE_CONFIG_STRING,
                                                               "Screen mode:",
                                                               DEFAULT_SCREEN_MODE_CONFIG_VALUE, gCurScreenMode,
                                                               screenModeMap,
                                                               &multiItemChanged));

        constexpr WUPSConfigItemMultipleValues::ValuePair audioModeMap[] = {
                {AUDIO_MODE_NONE, "Normal"},
                {AUDIO_MODE_SWAP, "Swap TV and GamePad sound"},
                {AUDIO_MODE_MATCH_SCREEN, "Sound matches screen"},
                {AUDIO_MODE_COMBINE, "Combine TV and GamePad sound"},
                {AUDIO_MODE_LEFT_TV_RIGHT_DRC, "Left: TV; Right: GamePad"}};

        root.add(WUPSConfigItemMultipleValues::CreateFromValue(AUDIO_MODE_CONFIG_STRING,
                                                               "Audio mode:",
                                                               DEFAULT_AUDIO_MODE_CONFIG_VALUE, gCurAudioMode,
                                                               audioModeMap,
                                                               &multiItemChanged));

        auto buttonCombos = WUPSConfigCategory::Create("Button Combos");

        buttonCombos.add(WUPSConfigItemBoolean::Create(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING,
                                                       "Enable swap screen button combo",
                                                       DEFAULT_ENABLED_SWAP_SCREENS_COMBO_CONFIG_VALUE, gSwapScreenButtonComboEnabled,
                                                       &boolItemChangedConfig));

        buttonCombos.add(WUPSConfigItemButtonCombo::Create(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING,
                                                           "Swap screen",
                                                           DEFAULT_SWAP_SCREEN_BUTTON_COMBO_CONFIG_VALUE, gSwapScreenButtonComboHandle,
                                                           &buttonComboItemChanged));

        buttonCombos.add(WUPSConfigItemBoolean::Create(ENABLED_CHANGE_SCREEN_COMBO_CONFIG_STRING,
                                                       "Enable change screen button combo",
                                                       DEFAULT_ENABLED_CHANGE_SCREEN_COMBO_CONFIG_VALUE, gChangeScreenModeButtonComboEnabled,
                                                       &boolItemChangedConfig));

        buttonCombos.add(WUPSConfigItemButtonCombo::Create(CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING,
                                                           "Change screen",
                                                           DEFAULT_CHANGE_SCREEN_BUTTON_COMBO_CONFIG_VALUE, gChangeScreenButtonComboHandle,
                                                           &buttonComboItemChanged));

        buttonCombos.add(WUPSConfigItemBoolean::Create(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING,
                                                       "Enable change audio mode button combo",
                                                       DEFAULT_ENABLED_CHANGE_AUDIO_COMBO_CONFIG_VALUE, gChangeAudioModeButtonComboEnabled,
                                                       &boolItemChangedConfig));

        buttonCombos.add(WUPSConfigItemButtonCombo::Create(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING,
                                                           "Change audio",
                                                           DEFAULT_CHANGE_AUDIO_BUTTON_COMBO_CONFIG_VALUE, gSwapAudioButtonComboHandle,
                                                           &buttonComboItemChanged));
        root.add(std::move(buttonCombos));
    } catch (std::exception &e) {
        OSReport("Exception: %s\n", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }

    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
    // Save all changes
    WUPSStorageError err;
    if ((err = WUPSStorageAPI::SaveStorage()) != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage: %s (%d)", WUPSStorageAPI_GetStatusStr(err), err);
    }
}