#include "retain_vars.hpp"
#include <vpad/input.h>

uint32_t gSwapScreenButtonCombo         = VPAD_BUTTON_TV;
uint32_t gSwapAudioButtonCombo          = VPAD_BUTTON_STICK_L;
bool gSwapScreenButtonComboEnabled      = true;
bool gChangeAudioModeButtonComboEnabled = false;
bool gEnabled                           = true;
bool gDoScreenSwap                      = false;
bool gShowNotifications                 = true;
bool gNotificationModuleInitDone        = true;
SwipSwapAudioMode gCurAudioMode         = AUDIO_MODE_MATCH_SCREEN;