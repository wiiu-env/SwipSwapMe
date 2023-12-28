#include "retain_vars.hpp"
#include "utils/config.h"

bool gEnabled                           = DEFAULT_ENABLED_CONFIG_VALUE;
bool gDoScreenSwap                      = DEFAULT_SWAP_SCREENS_CONFIG_VALUE;
uint32_t gSwapScreenButtonCombo         = DEFAULT_SWAP_SCREEN_BUTTON_COMBO_CONFIG_VALUE;
uint32_t gSwapAudioButtonCombo          = DEFAULT_CHANGE_AUDIO_BUTTON_COMBO_CONFIG_VALUE;
bool gSwapScreenButtonComboEnabled      = DEFAULT_ENABLED_SWAP_SCREENS_COMBO_CONFIG_VALUE;
bool gChangeAudioModeButtonComboEnabled = DEFAULT_ENABLED_CHANGE_AUDIO_COMBO_CONFIG_VALUE;
bool gShowNotifications                 = DEFAULT_ENABLE_NOTIFICATIONS_CONFIG_VALUE;
SwipSwapAudioMode gCurAudioMode         = DEFAULT_AUDIO_MODE_CONFIG_VALUE;

bool gNotificationModuleInitDone = true;