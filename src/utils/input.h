#pragma once

#include <cstdint>
#include <padscore/kpad.h>
#include <vpad/input.h>

#define VPAD_BUTTON_RESERVED_BIT 0x80000

uint32_t remapWiiMoteButtons(uint32_t buttons);
uint32_t remapClassicButtons(uint32_t buttons);
uint32_t remapProButtons(uint32_t buttons);

bool checkButtonComboVPAD(VPADStatus *buffer, uint32_t buffer_size, uint32_t buttonCombo, uint32_t &wasHoldForXFrames);
bool checkButtonComboWPAD(uint32_t vpadButtonHold, uint32_t vpadButtonCombo, uint32_t &wasHoldForXFrames);