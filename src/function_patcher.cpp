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
#include "retain_vars.hpp"
#include "utils/input.h"
#include "utils/logger.h"
#include <coreinit/dynload.h>
#include <gx2/surface.h>
#include <gx2/swap.h>
#include <memory/mappedmemory.h>
#include <notifications/notifications.h>
#include <padscore/wpad.h>
#include <vpad/input.h>
#include <wups.h>

void UpdateAudioMode();

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, GX2ColorBuffer *colorBuffer, GX2ScanTarget scan_target) {
    if (gEnabled && gDoScreenSwap) {
        if (scan_target == GX2_SCAN_TARGET_TV) {
            scan_target = GX2_SCAN_TARGET_DRC;
        } else if (scan_target == GX2_SCAN_TARGET_DRC) {
            scan_target = GX2_SCAN_TARGET_TV;
        }
        if (colorBuffer->surface.aa != GX2_AA_MODE1X) {
            // If AA is enabled, we need to resolve the AA buffer.
            GX2Surface tempSurface;
            tempSurface    = colorBuffer->surface;
            tempSurface.aa = GX2_AA_MODE1X;
            GX2CalcSurfaceSizeAndAlignment(&tempSurface);

            tempSurface.image = MEMAllocFromMappedMemoryForGX2Ex(tempSurface.imageSize, tempSurface.alignment);
            if (tempSurface.image != nullptr) {
                GX2ResolveAAColorBuffer(colorBuffer, &tempSurface, 0, 0);

                auto surfaceCpy      = colorBuffer->surface;
                colorBuffer->surface = tempSurface;
                real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
                colorBuffer->surface = surfaceCpy;

                if (tempSurface.image != nullptr) {
                    MEMFreeToMappedMemory(tempSurface.image);
                    tempSurface.image = nullptr;
                }
                return;
            } else {
                DEBUG_FUNCTION_LINE_ERR("Failed to allocate %d bytes for resolving AA", tempSurface.imageSize);
            }
        }
    }

    real_GX2CopyColorBufferToScanBuffer(colorBuffer, scan_target);
}

void SwapScreens() {
    gDoScreenSwap = !gDoScreenSwap;

    if (gShowNotifications && gNotificationModuleInitDone) {
        if (gDoScreenSwap) {
            NotificationModule_AddInfoNotification("Swapping TV and GamePad screen");
        } else {
            NotificationModule_AddInfoNotification("Stop swapping TV and GamePad screen");
        }
    }
}

void SwapVoices();
extern "C" uint32_t VPADGetButtonProcMode(VPADChan chan);

static uint32_t sSwapScreenWasHoldForXFrameGamePad = 0;
static uint32_t sSwapVoicesWasHoldForXFrameGamePad = 0;
DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus *buffer, uint32_t buffer_size, VPADReadError *error) {
    VPADReadError real_error;
    int32_t result = real_VPADRead(chan, buffer, buffer_size, &real_error);

    if (gEnabled && (gSwapScreenButtonComboEnabled || gChangeAudioModeButtonComboEnabled)) {
        if (result > 0 && real_error == VPAD_READ_SUCCESS) {
            // Fix games like TP HD
            bool checkFullBuffer = VPADGetButtonProcMode(chan) == 1;

            if (gChangeAudioModeButtonComboEnabled && checkButtonComboVPAD(buffer,
                                                                           checkFullBuffer ? result : 1,
                                                                           gSwapAudioButtonCombo,
                                                                           sSwapVoicesWasHoldForXFrameGamePad)) {
                SwapVoices();
            }

            if (gSwapScreenButtonComboEnabled && checkButtonComboVPAD(buffer,
                                                                      checkFullBuffer ? result : 1,
                                                                      gSwapScreenButtonCombo,
                                                                      sSwapScreenWasHoldForXFrameGamePad)) {
                SwapScreens();
            }
        }
    }

    if (error) {
        *error = real_error;
    }
    return result;
}

const char *modeToStr(SwipSwapAudioMode mode) {
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

void SwapVoices() {
    auto val = (uint32_t) gCurAudioMode;
    val++;
    if (val >= AUDIO_MODE_MAX_VALUE) {
        val = 0;
    }
    gCurAudioMode = static_cast<SwipSwapAudioMode>(val);
    if (gShowNotifications && gNotificationModuleInitDone) {
        NotificationModule_AddInfoNotification(modeToStr(gCurAudioMode));
    }
}

static uint32_t sSwapScreenWasHoldForXFrame[4];
static uint32_t sSwapAudioWasHoldForXFrame[4];
DECL_FUNCTION(void, WPADRead, WPADChan chan, WPADStatusProController *data) {
    real_WPADRead(chan, data);

    if (gEnabled && (gSwapScreenButtonComboEnabled || gChangeAudioModeButtonComboEnabled) && chan >= 0 && chan < 4) {
        if (data && data[0].err == 0) {
            if (data[0].extensionType != 0xFF) {
                uint32_t curButtonHold = 0;
                if (data[0].extensionType == WPAD_EXT_CORE || data[0].extensionType == WPAD_EXT_NUNCHUK) {
                    // button data is in the first 2 bytes for wiimotes
                    curButtonHold = remapWiiMoteButtons(((uint16_t *) data)[0]);
                } else if (data[0].extensionType == WPAD_EXT_CLASSIC) {
                    curButtonHold = remapClassicButtons(((uint32_t *) data)[10] & 0xFFFF);
                } else if (data[0].extensionType == WPAD_EXT_PRO_CONTROLLER) {
                    curButtonHold = remapProButtons(data[0].buttons);
                }

                if (gSwapScreenButtonComboEnabled && checkButtonComboWPAD(curButtonHold, gSwapScreenButtonCombo, sSwapScreenWasHoldForXFrame[chan])) {
                    SwapScreens();
                }
                if (gChangeAudioModeButtonComboEnabled && checkButtonComboWPAD(curButtonHold, gSwapAudioButtonCombo, sSwapAudioWasHoldForXFrame[chan])) {
                    SwapVoices();
                }
            }
        }
    }
}

int16_t DRCCopy[0x120] = {};
int16_t TVCopy[0x120]  = {};
typedef void (*AIInitDMAfn)(int16_t *addr, uint32_t size);
void DoAudioMagic(int16_t *addr, uint32_t size, bool isDRC, AIInitDMAfn targetFunc, AIInitDMAfn otherFunc) {
    auto sizeCpy = size > 576 ? 576 : size;
    if (!gEnabled) {
        targetFunc(addr, size);
        return;
    }
    switch (gCurAudioMode) {
        case AUDIO_MODE_NONE:
            break;
        case AUDIO_MODE_SWAP:
            otherFunc(addr, sizeCpy);
            return;
        case AUDIO_MODE_MATCH_SCREEN: {
            if (gDoScreenSwap) {
                otherFunc(addr, sizeCpy);
                return;
            }
            break;
        }
        case AUDIO_MODE_COMBINE:
        case AUDIO_MODE_LEFT_TV_RIGHT_DRC: {
            if (isDRC) {
                memcpy(DRCCopy, addr, sizeCpy);
            } else {
                memcpy(TVCopy, addr, sizeCpy);
            }
            if (gCurAudioMode == AUDIO_MODE_COMBINE) {
                for (uint32_t i = 0; i < 0x120; i += 2) {
                    // Combine left channel of TV and DRC
                    auto val = (((int32_t) TVCopy[i] + (int32_t) DRCCopy[i]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i] = (int16_t) val;
                    // Combine right channel of TV and DRC
                    val = (((int32_t) TVCopy[i] + (int32_t) DRCCopy[i]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i + 1] = (int16_t) val;
                }
            } else if (gCurAudioMode == AUDIO_MODE_LEFT_TV_RIGHT_DRC) {
                for (uint32_t i = 0; i < 0x120; i += 2) {
                    // Mix down TV to MONO and put it in the left channel
                    auto val = (((int32_t) TVCopy[i] + (int32_t) TVCopy[i + 1]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i] = (int16_t) val;
                    // Mix down DRC to MONO and put it in the right channel
                    val = (((int32_t) DRCCopy[i] + (int32_t) DRCCopy[i + 1]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i + 1] = (int16_t) val;
                }
            }
            break;
        }
        default:
            gCurAudioMode = AUDIO_MODE_NONE;
            break;
    }
    targetFunc(addr, sizeCpy);
}

extern "C" void (*real_AIInitDMA)(int16_t *addr, uint32_t size) __attribute__((section(".data")));
extern "C" void (*real_AIInitDMA2)(int16_t *addr, uint32_t size) __attribute__((section(".data")));

DECL_FUNCTION(void, AI2InitDMA, int16_t *addr, uint32_t size) {
    DoAudioMagic(addr, size, true, real_AI2InitDMA, real_AIInitDMA);
}
DECL_FUNCTION(void, AIInitDMA, int16_t *addr, uint32_t size) {
    DoAudioMagic(addr, size, false, real_AIInitDMA, real_AI2InitDMA);
}

DECL_FUNCTION(void, AI2InitDMA2, int16_t *addr, uint32_t size) {
    DoAudioMagic(addr, size, true, real_AI2InitDMA2, real_AIInitDMA2);
}
DECL_FUNCTION(void, AIInitDMA2, int16_t *addr, uint32_t size) {
    DoAudioMagic(addr, size, false, real_AIInitDMA2, real_AI2InitDMA2);
}

bool sAvoidRecursiveCall = false;
DECL_FUNCTION(void, AXUpdateDeviceModes) {
    if (!gEnabled) {
        real_AXUpdateDeviceModes();
        return;
    }
    sAvoidRecursiveCall = true;
    real_AXUpdateDeviceModes();
    sAvoidRecursiveCall = false;
}
bool sAvoidRecursiveCall2 = false;
DECL_FUNCTION(void, AXUpdateDeviceModes2) {
    if (!gEnabled) {
        real_AXUpdateDeviceModes2();
        return;
    }
    sAvoidRecursiveCall2 = true;
    real_AXUpdateDeviceModes2();
    sAvoidRecursiveCall2 = false;
}

DECL_FUNCTION(uint32_t, AVMGetTVAudioMode, uint32_t *mode) {
    auto res = real_AVMGetTVAudioMode(mode);
    if (!gEnabled) {
        return res;
    }
    if (*mode != 1) {
        // force stereo
        *mode = 1;
        UpdateAudioMode();
    }

    return res;
}

void UpdateAudioMode() {
    OSDynLoad_Module mod = nullptr;
    if (OSDynLoad_IsModuleLoaded("sndcore2", &mod) == OS_DYNLOAD_OK && mod != nullptr) {
        if (real_AXUpdateDeviceModes2 != nullptr && !sAvoidRecursiveCall2) {
            my_AXUpdateDeviceModes2();
        }
    } else if (OSDynLoad_IsModuleLoaded("snd_core", &mod) == OS_DYNLOAD_OK && mod != nullptr) {
        if (real_AXUpdateDeviceModes != nullptr && !sAvoidRecursiveCall) {
            my_AXUpdateDeviceModes();
        }
    }
}

WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
WUPS_MUST_REPLACE(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead);
WUPS_MUST_REPLACE(WPADRead, WUPS_LOADER_LIBRARY_PADSCORE, WPADRead);

WUPS_MUST_REPLACE_FOR_PROCESS(AIInitDMA, WUPS_LOADER_LIBRARY_SND_CORE, AIInitDMA, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AI2InitDMA, WUPS_LOADER_LIBRARY_SND_CORE, AI2InitDMA, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AIInitDMA2, WUPS_LOADER_LIBRARY_SNDCORE2, AIInitDMA, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AI2InitDMA2, WUPS_LOADER_LIBRARY_SNDCORE2, AI2InitDMA, WUPS_FP_TARGET_PROCESS_ALL);

WUPS_MUST_REPLACE_FOR_PROCESS(AVMGetTVAudioMode, WUPS_LOADER_LIBRARY_AVM, AVMGetTVAudioMode, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AXUpdateDeviceModes, WUPS_LOADER_LIBRARY_SND_CORE, AXUpdateDeviceModes, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AXUpdateDeviceModes2, WUPS_LOADER_LIBRARY_SNDCORE2, AXUpdateDeviceModes, WUPS_FP_TARGET_PROCESS_ALL);
