#include "config.h"
#include "WUPSConfigItemButtonCombo.h"
#include "retain_vars.hpp"
#include <map>
#include <wups/config/WUPSConfigItemMultipleValues.h>

extern void UpdateAudioMode();

void boolItemChangedConfig(ConfigItemBoolean *item, bool newValue) {
    if (std::string_view(ENABLED_CONFIG_STRING) == item->identifier) {
        DEBUG_FUNCTION_LINE("New value in %s: %d", ENABLED_CONFIG_STRING, newValue);
        gEnabled = newValue;
        WUPSStorageAPI::Store(ENABLED_CONFIG_STRING, gEnabled);
        UpdateAudioMode();
        return;
    }

    if (std::string_view(SWAP_SCREENS_CONFIG_STRING) == item->identifier) {
        gDoScreenSwap = newValue;
        WUPSStorageAPI::Store(item->identifier, gDoScreenSwap);
    } else if (std::string_view(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING) == item->identifier) {
        gSwapScreenButtonComboEnabled = newValue;
        WUPSStorageAPI::Store(item->identifier, gSwapScreenButtonComboEnabled);
    } else if (std::string_view(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING) == item->identifier) {
        gChangeAudioModeButtonComboEnabled = newValue;
        WUPSStorageAPI::Store(item->identifier, gChangeAudioModeButtonComboEnabled);
    } else if (std::string_view(ENABLE_NOTIFICATIONS_CONFIG_STRING) == item->identifier) {
        gShowNotifications = newValue;
        WUPSStorageAPI::Store(item->identifier, gShowNotifications);
    } else {
        DEBUG_FUNCTION_LINE_ERR("Unexpected boolean item: %s", item->identifier);
    }
}

void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    if (item && item->identifier) {
        if (std::string_view(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING) == item->identifier) {
            gSwapScreenButtonCombo = newValue;
            WUPSStorageAPI::Store(item->identifier, (int32_t) gSwapScreenButtonCombo);
        } else if (std::string_view(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING) == item->identifier) {
            gSwapAudioButtonCombo = newValue;
            WUPSStorageAPI::Store(item->identifier, (int32_t) gSwapAudioButtonCombo);
        }
    }
}


void default_audio_mode_changed(ConfigItemMultipleValues *item, uint32_t newValue) {
    DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->identifier, newValue);

    if (std::string_view(AUDIO_MODE_CONFIG_STRING) == item->identifier) {
        gCurAudioMode = static_cast<SwipSwapAudioMode>(newValue);
        WUPSStorageAPI::Store(item->identifier, (int32_t) gCurAudioMode);
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
        root.add(WUPSConfigItemBoolean::Create(SWAP_SCREENS_CONFIG_STRING,
                                               "Swap screens",
                                               DEFAULT_SWAP_SCREENS_CONFIG_VALUE, gDoScreenSwap,
                                               &boolItemChangedConfig));

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
                                                               &default_audio_mode_changed));

        auto buttonCombos = WUPSConfigCategory::Create("Button Combos");

        buttonCombos.add(WUPSConfigItemBoolean::Create(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING,
                                                       "Enable swap screen button combo",
                                                       DEFAULT_ENABLED_SWAP_SCREENS_COMBO_CONFIG_VALUE, gSwapScreenButtonComboEnabled,
                                                       &boolItemChangedConfig));

        buttonCombos.add(WUPSConfigItemButtonCombo::Create(SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING,
                                                           "Swap screen",
                                                           DEFAULT_SWAP_SCREEN_BUTTON_COMBO_CONFIG_VALUE, gSwapScreenButtonCombo,
                                                           &buttonComboItemChanged));

        buttonCombos.add(WUPSConfigItemBoolean::Create(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING,
                                                       "Enable change audio mode button combo",
                                                       DEFAULT_ENABLED_CHANGE_AUDIO_COMBO_CONFIG_VALUE, gChangeAudioModeButtonComboEnabled,
                                                       &boolItemChangedConfig));

        buttonCombos.add(WUPSConfigItemButtonCombo::Create(CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING,
                                                           "Change audio",
                                                           DEFAULT_CHANGE_AUDIO_BUTTON_COMBO_CONFIG_VALUE, gSwapAudioButtonCombo,
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
    if (WUPSStorageAPI::SaveStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}