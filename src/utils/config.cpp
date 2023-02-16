#include "config.h"
#include "WUPSConfigItemButtonCombo.h"
#include "retain_vars.hpp"
#include <map>
#include <wups/config/WUPSConfigItemMultipleValues.h>
extern void UpdateAudioMode();

void boolItemChangedConfig(ConfigItemBoolean *item, bool newValue) {
    if (std::string_view(item->configId) == ENABLED_CONFIG_STRING) {
        DEBUG_FUNCTION_LINE("New value in %s: %d", ENABLED_CONFIG_STRING, newValue);
        gEnabled = newValue;
        WUPS_StoreInt(nullptr, ENABLED_CONFIG_STRING, gEnabled);
        UpdateAudioMode();
        return;
    }
    PROCESS_BOOL_ITEM_CHANGED(SWAP_SCREENS_CONFIG_STRING, gDoScreenSwap);
    PROCESS_BOOL_ITEM_CHANGED(ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING, gSwapScreenButtonComboEnabled);
    PROCESS_BOOL_ITEM_CHANGED(ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING, gChangeAudioModeButtonComboEnabled);
    PROCESS_BOOL_ITEM_CHANGED(ENABLE_NOTIFICATIONS_CONFIG_STRING, gShowNotifications);
}

void buttonComboItemChanged(ConfigItemButtonCombo *item, uint32_t newValue) {
    if (item && item->configId) {
        if (std::string_view(item->configId) == SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING) {
            gSwapScreenButtonCombo = newValue;
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gSwapScreenButtonCombo);
        } else if (std::string_view(item->configId) == CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING) {
            gSwapAudioButtonCombo = newValue;
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gSwapAudioButtonCombo);
        }
    }
}

void getConfigInfoForMap(std::map<SwipSwapAudioMode, const char *> &curMap, ConfigItemMultipleValuesPair *pair, uint32_t default_lang, uint32_t *default_index, uint32_t *len) {
    uint32_t i = 0;
    for (auto &curEntry : curMap) {
        if (default_lang == curEntry.first) {
            *default_index = i;
        }
        pair[i].value     = curEntry.first;
        pair[i].valueName = (char *) curEntry.second;
        i++;
    }
    *len = i;
}


void default_lang_changed(ConfigItemMultipleValues *item, uint32_t newValue) {
    DEBUG_FUNCTION_LINE("New value in %s changed: %d", item->configId, newValue);

    if (strcmp(item->configId, SCREEN_MODE_CONFIG_STRING) == 0) {
        gCurAudioMode = static_cast<SwipSwapAudioMode>(newValue);
        WUPS_StoreInt(nullptr, item->configId, (int32_t) gCurAudioMode);
    }
}

WUPS_GET_CONFIG() {
    // We open the storage, so we can persist the configuration the user did.
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to open storage");
        return 0;
    }

    WUPSConfigHandle config;
    WUPSConfig_CreateHandled(&config, "SwipSwapMe");

    WUPSConfigCategoryHandle setting;
    WUPSConfig_AddCategoryByNameHandled(config, "Settings", &setting);
    WUPSConfigCategoryHandle combos;
    WUPSConfig_AddCategoryByNameHandled(config, "Button combos", &combos);

    std::map<SwipSwapAudioMode, const char *> audioModeMap{
            {AUDIO_MODE_NONE, "Normal"},
            {AUDIO_MODE_SWAP, "Swap TV and GamePad sound"},
            {AUDIO_MODE_MATCH_SCREEN, "Sound matches screen"},
            {AUDIO_MODE_COMBINE, "Combine TV and GamePad sound"},
            {AUDIO_MODE_LEFT_TV_RIGHT_DRC, "Left: TV; Right: GamePad"}};

    ConfigItemMultipleValuesPair audioModePairs[audioModeMap.size()];

    uint32_t number_values = 0;
    uint32_t default_index = 0;
    getConfigInfoForMap(audioModeMap, audioModePairs, gCurAudioMode, &default_index, &number_values);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, setting, ENABLED_CONFIG_STRING, "Plugin enabled", gEnabled, &boolItemChangedConfig);
    WUPSConfigItemBoolean_AddToCategoryHandled(config, setting, ENABLE_NOTIFICATIONS_CONFIG_STRING, "Show notifications", gShowNotifications, &boolItemChangedConfig);
    WUPSConfigItemBoolean_AddToCategoryHandled(config, setting, SWAP_SCREENS_CONFIG_STRING, "Swap screens", gDoScreenSwap, &boolItemChangedConfig);
    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, setting, SCREEN_MODE_CONFIG_STRING, "Audio mode:", default_index, audioModePairs, number_values,
                                                      &default_lang_changed);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, combos, ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING, "Enable swap screen button combo", gSwapScreenButtonComboEnabled, &boolItemChangedConfig);
    WUPSConfigItemButtonCombo_AddToCategoryHandled(config, combos, SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING, "Swap screen", gSwapScreenButtonCombo, &buttonComboItemChanged);
    WUPSConfigItemBoolean_AddToCategoryHandled(config, combos, ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING, "Enable change audio mode button combo", gChangeAudioModeButtonComboEnabled, &boolItemChangedConfig);
    WUPSConfigItemButtonCombo_AddToCategoryHandled(config, combos, CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING, "Change audio", gSwapAudioButtonCombo, &buttonComboItemChanged);

    return config;
}

WUPS_CONFIG_CLOSED() {
    // Save all changes
    if (WUPS_CloseStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to close storage");
    }
}