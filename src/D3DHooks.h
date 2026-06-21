#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <cstdio>
#include "libs/MinHook/include/MinHook.h"
#include "src/Utils.h"
#include "src/TypeDefs.h"

namespace D3D {
    void HookD3D9(HWND hwnd);

    //not the best place to put these
    //dont need all of these but they could be useful later
    static constexpr DWORD kTechlandWindowed_Style   = 0x90C00000; // WS_POPUP | WS_VISIBLE | WS_CAPTION
    static constexpr DWORD kTechlandWindowed_ExStyle  = 0x00040000; // WS_EX_APPWINDOW
    static constexpr DWORD kTechlandFullscreen_Style  = 0x90000000; // WS_POPUP | WS_VISIBLE
    static constexpr DWORD kTechlandFullscreen_ExStyle = 0x00000008; // WS_EX_TOPMOST

    //this is the styling we apply
    static constexpr DWORD kDesiredStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

}
