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

typedef enum SwipSwapScreenMode {
    SCREEN_MODE_NONE       = 0,
    SCREEN_MODE_SWAP       = 1,
    SCREEN_MODE_MIRROR_TV  = 2,
    SCREEN_MODE_MIRROR_DRC = 3,
    SCREEN_MODE_MAX_VALUE  = 4
} SwipSwapScreenMode;

extern uint32_t gSwapScreenButtonCombo;
extern uint32_t gSwapAudioButtonCombo;
extern bool gSwapScreenButtonComboEnabled;
extern bool gChangeAudioModeButtonComboEnabled;
extern bool gEnabled;
extern bool gShowNotifications;
extern bool gNotificationModuleInitDone;

extern SwipSwapScreenMode gCurScreenMode;
extern SwipSwapAudioMode gCurAudioMode;