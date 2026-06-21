#include <Windows.h>
#include <vector>
#include <cstdint>
#include <winternl.h>
#include <src/Utils.h>

#pragma comment(lib, "ntdll.lib")

namespace Utils {

    void InitialiseConsole()
    {
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        SetConsoleTitleA("BanoiPatch");
    }

    uintptr_t FindPattern(HMODULE hModule, const char* signature)
    {
        static auto patternToByte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current) {
                if (*current == '?') {
                    ++current;
                    if (*current == '?') ++current;
                    bytes.push_back(-1);
                }
                else {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)hModule;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)hModule + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = patternToByte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(hModule);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i) {
            bool found = true;
            for (auto j = 0ul; j < s; ++j) {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }
            if (found) return reinterpret_cast<uintptr_t>(&scanBytes[i]);
        }
        return 0;
    }

    bool IsWindows7SP1OrNewer() { //maybe could be better to use one of the actual windows function
        using RtlGetVersion_t = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);

        HMODULE ntDLL = GetModuleHandleW(L"ntdll.dll");
        if (!ntDLL)
            return false;

        auto RtlGetVersion = reinterpret_cast<RtlGetVersion_t>(GetProcAddress(ntDLL, "RtlGetVersion"));
        if (!RtlGetVersion)
            return false;

        RTL_OSVERSIONINFOEXW version {};
        version.dwOSVersionInfoSize = sizeof(version);

        if (RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&version)) != 0)
            return false;

        //Windows 8 and above
        if (version.dwMajorVersion > 6)
            return true;

        //Windows 7/Vista/XP
        if (version.dwMajorVersion == 6 && version.dwMinorVersion > 1)
            return true;

        return version.dwMajorVersion == 6 && version.dwMinorVersion == 1 && version.wServicePackMajor >= 1;
    }

    void PatchMemory(uintptr_t address, const uint8_t* bytes, size_t size)
    {
        DWORD oldProtect;
        //Unprotect the memory to allow writing
        VirtualProtect(reinterpret_cast<void*>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect);
        //Write our new bytes
        memcpy(reinterpret_cast<void*>(address), bytes, size);
        //Restore the original memory protection
        VirtualProtect(reinterpret_cast<void*>(address), size, oldProtect, &oldProtect);
    }
}
