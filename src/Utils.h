#pragma once
#include <Windows.h>
#include <cstdint>

namespace Utils {
    void InitialiseConsole();
    uintptr_t FindPattern(HMODULE hModule, const char* signature);
    bool IsWindows7SP1OrNewer();
    void PatchMemory(uintptr_t address, const uint8_t* bytes, size_t size);
}

#define DEBUG_LOG(fmt, ...) \
do { printf("[BanoiPatch] " fmt "\n", ##__VA_ARGS__); } while(0)

/*
#ifdef _DEBUG
    #define DEBUG_LOG(fmt, ...) \
    do { printf("[BanoiPatch] " fmt "\n", ##__VA_ARGS__); } while(0)
#else
    #define DEBUG_LOG(fmt, ...) ((void)0)
#endif */