// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"

HMODULE dll_handle = 0;
uint16_t* skillIDReplacement = nullptr;
SkillHotLoop oSkillHotLoop;

struct SkillPair
{
	short skillOriginal;
	short skillOverride;
};

intptr_t SkillHotLoopHook(intptr_t thisPTR, intptr_t param2)
{
    *skillIDReplacement = *(uint16_t*)(thisPTR + 0x79c8);
    *skillIDReplacement = 10;

    return oSkillHotLoop(thisPTR, param2);
}

void __stdcall ModStart()
{
    std::string pattern;
    pattern = "45 0F B7 86 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8D 8D ?? ?? ?? ?? E8 ?? ?? ?? ?? 44 38 BD ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 45 33 C9 48 8D 8D ?? ?? ?? ?? 45 33 C0 33 D2 E8 ?? ?? ?? ?? 48 8B D8 48 85 C0 75 ?? 49 8B FF";
    uintptr_t instructionAddress = PatternScan(GetModuleHandle(NULL), pattern);

    pattern = "45 0F B7 86 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8D 8D ?? ?? ?? ?? E8 ?? ?? ?? ?? 44 38 BD ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 45 33 C9 48 8D 8D ?? ?? ?? ?? 45 33 C0 33 D2 E8 ?? ?? ?? ?? 48 8B D8 48 85 C0 75 ?? 4D 8B E7";
    uintptr_t secondAddress = PatternScan(GetModuleHandle(NULL), pattern);

    pattern = "48 8D 0D ?? ?? ?? ?? 48 69 C0 A0 02 00 00 48 01 C8";
    uintptr_t unitArray = PatternScan(GetModuleHandle(NULL), pattern);
    unitArray = GetAddressFromGlobalRef(unitArray);

    skillIDReplacement = (uint16_t*)unitArray;
    int32_t offset = (int32_t)(unitArray - (instructionAddress + 8));
    int32_t offset2 = (int32_t)(unitArray - (secondAddress + 8));

	std::array<BYTE,8> patch = 
    {
	    0x4C,
	    0x0F,
		0xB7,
		0x05,
		(BYTE)(offset),
		(BYTE)(offset >> 8),
		(BYTE)(offset >> 16),
		(BYTE)(offset >> 24)
	};
	std::array<BYTE,8> patch2 = 
    {
	    0x4C,
	    0x0F,
		0xB7,
		0x05,
		(BYTE)(offset2),
		(BYTE)(offset2 >> 8),
		(BYTE)(offset2 >> 16),
		(BYTE)(offset2 >> 24)
	};

    //BYTE filler = 0x00;

    DWORD old;
	VirtualProtect((void*)instructionAddress, 8, PAGE_EXECUTE_READWRITE, &old);
	memcpy((void*)instructionAddress, patch.data(), sizeof(patch));
	VirtualProtect((void*)instructionAddress, 8, old, &old);

	VirtualProtect((void*)secondAddress, 8, PAGE_EXECUTE_READWRITE, &old);
	memcpy((void*)secondAddress, patch2.data(), sizeof(patch2));
	VirtualProtect((void*)secondAddress, 8, old, &old);

    pattern = "48 8D 05 ?? ?? ?? ?? 48 89 44 24 ?? 48 8D 54 24 ?? 33 C0 48 8B CE";
    uintptr_t skillHotLoopAddress = PatternScan(GetModuleHandle(NULL), pattern);
    skillHotLoopAddress = GetAddressFromGlobalRef(skillHotLoopAddress);
    oSkillHotLoop = (SkillHotLoop)skillHotLoopAddress;

    printf("Addresses: 0x%p, 0x%p, 0x%p\n", instructionAddress, skillIDReplacement, oSkillHotLoop);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)oSkillHotLoop, SkillHotLoopHook);
    DetourTransactionCommit();
}

extern "C" __declspec(dllexport) void Init()
{
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ModStart, 0, 0, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dll_handle = hModule;
        FILE* newOut;
        freopen_s(&newOut, "CONOUT$", "w", stdout);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

