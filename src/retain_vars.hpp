#pragma once

#include <cstdint>

typedef enum SwipSwapAudioMode {
    AUDIO_MODE_NONE              = 0,
    AUDIO_MODE_SWAP              = 1,
    AUDIO_MODE_MATCH_SCREEN      = 2,
    AUDIO_MODE_COMBINE           = 3,
    AUDIO_MODE_LEFT_TV_RIGHT_DRC = 4,
    AUDIO_MODE_MAX_VALUE         = 5
} SwipSwapAudioMode;

extern uint32_t gSwapScreenButtonCombo;
extern uint32_t gSwapAudioButtonCombo;
extern bool gSwapScreenButtonComboEnabled;
extern bool gChangeAudioModeButtonComboEnabled;
extern bool gEnabled;
extern bool gDoScreenSwap;
extern bool gShowNotifications;
extern bool gNotificationModuleInitDone;
extern SwipSwapAudioMode gCurAudioMode;