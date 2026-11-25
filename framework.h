#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <detours.h>
#include "signaturescan.h"
#include "config_parser.h"
#include <array>
#include <filesystem>

extern uint16_t* skillIDReplacement;

typedef intptr_t(*SkillHotLoop)(intptr_t thisPTR, intptr_t param2);
extern SkillHotLoop oSkillHotLoop;