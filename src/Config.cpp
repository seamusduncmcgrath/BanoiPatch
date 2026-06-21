//
// Created by Seamus on 20/06/2026.
//
#include <Windows.h>
#include <iostream>
#include <string>
#include "src/Config.h"

namespace Config {
    float FOVMultiplier = 1.0f;
    bool enableDevMenu = true;
    bool ForceSOAllShadowMaps = true;
    bool SetScreenshotMode = false;
    bool FixWindowed = false;
    void Load() {
        char configPath[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, configPath);
        strcat_s(configPath, "\\BanoiPatch.ini");

        enableDevMenu = GetPrivateProfileIntA("Settings", "EnableDevMenu", 1, configPath) != 0;
        ForceSOAllShadowMaps = GetPrivateProfileIntA("Settings", "ImprovedShadows", 1, configPath) != 0;
        SetScreenshotMode = GetPrivateProfileIntA("Settings", "ImproveDrawDistance", 0, configPath) != 0;
        FixWindowed = GetPrivateProfileIntA("Settings", "FixWindowed", 0, configPath) != 0;

        char fovStr[32];
        GetPrivateProfileStringA("Settings", "FOVMultiplier", "1.0", fovStr, sizeof(fovStr), configPath);
        FOVMultiplier = std::stof(fovStr);

    }
}
