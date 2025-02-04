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
    if (gEnabled && gCurScreenMode != SCREEN_MODE_NONE) {
        switch (gCurScreenMode) {
            case SCREEN_MODE_SWAP: {
                if (scan_target == GX2_SCAN_TARGET_TV) {
                    scan_target = GX2_SCAN_TARGET_DRC;
                } else if (scan_target == GX2_SCAN_TARGET_DRC) {
                    scan_target = GX2_SCAN_TARGET_TV;
                }
                break;
            }
            case SCREEN_MODE_MIRROR_TV: {
                if (scan_target == GX2_SCAN_TARGET_TV) {
                    scan_target = GX2_SCAN_TARGET_TV | GX2_SCAN_TARGET_DRC;
                } else if (scan_target == GX2_SCAN_TARGET_DRC) {
                    return;
                }
                break;
            }
            case SCREEN_MODE_MIRROR_DRC: {
                if (scan_target == GX2_SCAN_TARGET_DRC) {
                    scan_target = GX2_SCAN_TARGET_TV | GX2_SCAN_TARGET_DRC;
                } else if (scan_target == GX2_SCAN_TARGET_TV) {
                    return;
                }
                break;
            }
            default:
                break;
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

static int16_t sDRCCopy[0x120] = {};
static int16_t sTVCopy[0x120]  = {};
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
            switch (gCurScreenMode) {
                case SCREEN_MODE_SWAP:
                    otherFunc(addr, sizeCpy);
                    return;
                case SCREEN_MODE_MIRROR_TV:
                case SCREEN_MODE_MIRROR_DRC: {
                    if (isDRC) {
                        memcpy(sDRCCopy, addr, sizeCpy);
                    } else {
                        memcpy(sTVCopy, addr, sizeCpy);
                    }
                    if (gCurScreenMode == SCREEN_MODE_MIRROR_TV) {
                        memcpy(addr, sTVCopy, sizeCpy);
                    } else if (gCurScreenMode == SCREEN_MODE_MIRROR_DRC) {
                        memcpy(addr, sDRCCopy, sizeCpy);
                    }
                    break;
                }
                case SCREEN_MODE_NONE:
                case SCREEN_MODE_MAX_VALUE:
                    break;
            }
            break;
        }
        case AUDIO_MODE_COMBINE:
        case AUDIO_MODE_LEFT_TV_RIGHT_DRC: {
            if (isDRC) {
                memcpy(sDRCCopy, addr, sizeCpy);
            } else {
                memcpy(sTVCopy, addr, sizeCpy);
            }
            if (gCurAudioMode == AUDIO_MODE_COMBINE) {
                for (uint32_t i = 0; i < 0x120; i += 2) {
                    // Combine left channel of TV and DRC
                    auto val = (((int32_t) sTVCopy[i] + (int32_t) sDRCCopy[i]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i] = (int16_t) val;
                    // Combine right channel of TV and DRC
                    val = (((int32_t) sTVCopy[i + 1] + (int32_t) sDRCCopy[i + 1]) >> 1);
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
                    auto val = (((int32_t) sTVCopy[i] + (int32_t) sTVCopy[i + 1]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i + 1] = (int16_t) val;
                    // Mix down DRC to MONO and put it in the right channel
                    val = (((int32_t) sDRCCopy[i] + (int32_t) sDRCCopy[i + 1]) >> 1);
                    if (val > 0x7FFF) {
                        val = 0x7FFF;
                    } else if (val < -0x8000) {
                        val = 0x8000;
                    }
                    addr[i] = (int16_t) val;
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
    const auto res = real_AVMGetTVAudioMode(mode);
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

WUPS_MUST_REPLACE_FOR_PROCESS(AIInitDMA, WUPS_LOADER_LIBRARY_SND_CORE, AIInitDMA, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AI2InitDMA, WUPS_LOADER_LIBRARY_SND_CORE, AI2InitDMA, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AIInitDMA2, WUPS_LOADER_LIBRARY_SNDCORE2, AIInitDMA, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AI2InitDMA2, WUPS_LOADER_LIBRARY_SNDCORE2, AI2InitDMA, WUPS_FP_TARGET_PROCESS_ALL);

WUPS_MUST_REPLACE_FOR_PROCESS(AVMGetTVAudioMode, WUPS_LOADER_LIBRARY_AVM, AVMGetTVAudioMode, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AXUpdateDeviceModes, WUPS_LOADER_LIBRARY_SND_CORE, AXUpdateDeviceModes, WUPS_FP_TARGET_PROCESS_ALL);
WUPS_MUST_REPLACE_FOR_PROCESS(AXUpdateDeviceModes2, WUPS_LOADER_LIBRARY_SNDCORE2, AXUpdateDeviceModes, WUPS_FP_TARGET_PROCESS_ALL);
