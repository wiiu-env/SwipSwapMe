#pragma once

#include <wups/button_combo/defines.h>
#include <wups/config.h>

#define DEFAULT_ENABLED_CONFIG_VALUE                        true
#define DEFAULT_ENABLED_SWAP_SCREENS_COMBO_CONFIG_VALUE     true
#define DEFAULT_ENABLED_CHANGE_SCREEN_COMBO_CONFIG_VALUE    false
#define DEFAULT_ENABLED_CHANGE_AUDIO_COMBO_CONFIG_VALUE     false
#define DEFAULT_ENABLE_NOTIFICATIONS_CONFIG_VALUE           true
#define DEFAULT_SWAP_SCREEN_BUTTON_COMBO_CONFIG_VALUE       ((WUPSButtonCombo_Buttons) (WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_RIGHT))
#define DEFAULT_CHANGE_AUDIO_BUTTON_COMBO_CONFIG_VALUE      ((WUPSButtonCombo_Buttons) (WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_STICK_L))
#define DEFAULT_CHANGE_SCREEN_BUTTON_COMBO_CONFIG_VALUE     ((WUPSButtonCombo_Buttons) (WUPS_BUTTON_COMBO_BUTTON_TV | WUPS_BUTTON_COMBO_BUTTON_STICK_R))

#define DEFAULT_SCREEN_MODE_CONFIG_VALUE                    SCREEN_MODE_NONE
#define DEFAULT_AUDIO_MODE_CONFIG_VALUE                     AUDIO_MODE_MATCH_SCREEN

#define ENABLED_CONFIG_STRING                               "enabled"
#define SWAP_SCREENS_CONFIG_STRING_DEPRECATED               "swapScreens"
#define SCREEN_MODE_CONFIG_STRING                           "screenMode"
#define ENABLED_SWAP_SCREENS_COMBO_CONFIG_STRING            "swapScreensComboEnabled"
#define ENABLED_CHANGE_SCREEN_COMBO_CONFIG_STRING           "changeScreenComboEnabled"
#define ENABLED_CHANGE_AUDIO_COMBO_CONFIG_STRING            "changeAudioComboEnabled"
#define ENABLE_NOTIFICATIONS_CONFIG_STRING                  "notificationsEnabled"
#define SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED   "screenButtonCombo"
#define CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING_DEPRECATED "screenChangeButtonCombo"
#define CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING_DEPRECATED  "audioButtonCombo"
#define SWAP_SCREEN_BUTTON_COMBO_CONFIG_STRING              "screenButtonComboNew"
#define CHANGE_SCREEN_BUTTON_COMBO_CONFIG_STRING            "screenChangeButtonComboNew"
#define CHANGE_AUDIO_BUTTON_COMBO_CONFIG_STRING             "audioButtonComboNew"
#define AUDIO_MODE_CONFIG_STRING                            "audioMode"


WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle);

void ConfigMenuClosedCallback();
