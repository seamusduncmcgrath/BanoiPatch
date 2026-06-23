#include <windows.h>
#include <dinput.h>
#include <stdio.h>

#include "libs/MinHook/include/MinHook.h"
#include "SDL3/SDL.h"
#include "src/TypeDefs.h"
#include "src/Utils.h"
#include "src/D3DHooks.h"
#include "src/Config.h"

XInputGetState_t        oXInputGetState        = nullptr;
XInputSetState_t        oXInputSetState        = nullptr;
XInputGetCapabilities_t oXInputGetCapabilities = nullptr;
SDL_Gamepad* g_CurrentGamepad = nullptr;
CreateMasteringVoice_t oCreateMasteringVoice = nullptr;
CoCreateInstance_t oCoCreateInstance = nullptr;
IsXInputDevice_t oIsXInputDevice = nullptr;
SetFOV_t oSetFOV = nullptr;
ShowLevelAfterFade_t oShowLevelAfterFade = nullptr;
ExecuteCommand_t pExecuteCommand = nullptr;
TakeScreenshot_t oTakeScreenshot = nullptr;
StartAVIRecording_t oStartAVIRecording = nullptr;
DirectInput8Create_t oDirectInput8Create = nullptr;
EnumDevices_t oEnumDevices = nullptr;
EnableCommentRender_t oEnableCommentRender = nullptr;
CheckCrcOpenedFile_t oCheckCrcOpenedFile = nullptr;
SetPlatform_t oSetPlatform = nullptr;
SetWindowLongA_t oSetWindowLongA = nullptr;
CreateWindowExW_t oCreateWindowExW = nullptr;
tCheckZipCRC oCheckZipCRC;
tCUIFrame oCUIFrame = nullptr;
void* pEngineConsole = nullptr;
IGame* g_IGame = nullptr;
CGame* g_Game = nullptr;

HWND g_GameHwnd = nullptr;
bool g_IsWindowed = false;

static const GUID CLSID_XAudio2       = {0x5a508685, 0xa254, 0x4fba, {0x9b,0x82,0x9a,0x24,0xb0,0x03,0x06,0xaf}};
static const GUID CLSID_XAudio2_Debug = {0xdb05ea35, 0x0329, 0x4d4b, {0xa5,0x3a,0x6d,0xea,0xd0,0x3d,0x38,0x52}};

bool g_bForceOrtho = false;

extern "C" DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
    if (dwUserIndex == 0 && g_CurrentGamepad != nullptr && SDL_GamepadConnected(g_CurrentGamepad))
    {
        memset(pState, 0, sizeof(XINPUT_STATE));

        static DWORD s_PacketNumber = 0;
        pState->dwPacketNumber = s_PacketNumber++;

        // buttons
        WORD buttons = 0;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_SOUTH))          buttons |= XINPUT_GAMEPAD_A;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_EAST))           buttons |= XINPUT_GAMEPAD_B;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_WEST))           buttons |= XINPUT_GAMEPAD_X;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_NORTH))          buttons |= XINPUT_GAMEPAD_Y;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))        buttons |= XINPUT_GAMEPAD_DPAD_UP;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))      buttons |= XINPUT_GAMEPAD_DPAD_DOWN;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))      buttons |= XINPUT_GAMEPAD_DPAD_LEFT;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))     buttons |= XINPUT_GAMEPAD_DPAD_RIGHT;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER))  buttons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) buttons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_START))          buttons |= XINPUT_GAMEPAD_START;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_BACK))           buttons |= XINPUT_GAMEPAD_BACK;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK))     buttons |= XINPUT_GAMEPAD_LEFT_THUMB;
        if (SDL_GetGamepadButton(g_CurrentGamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK))    buttons |= XINPUT_GAMEPAD_RIGHT_THUMB;
        pState->Gamepad.wButtons = buttons;

        // triggers (SDL int16 0-32767 -> XInput byte 0-255)
        int16_t ltSDL = SDL_GetGamepadAxis(g_CurrentGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        int16_t rtSDL = SDL_GetGamepadAxis(g_CurrentGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
        pState->Gamepad.bLeftTrigger  = static_cast<BYTE>((ltSDL / 32767.0f) * 255.0f);
        pState->Gamepad.bRightTrigger = static_cast<BYTE>((rtSDL / 32767.0f) * 255.0f);

        // sticks (SDL Y is inverted vs XInput)
        pState->Gamepad.sThumbLX = SDL_GetGamepadAxis(g_CurrentGamepad, SDL_GAMEPAD_AXIS_LEFTX);
        pState->Gamepad.sThumbRX = SDL_GetGamepadAxis(g_CurrentGamepad, SDL_GAMEPAD_AXIS_RIGHTX);

        int16_t lySDL = SDL_GetGamepadAxis(g_CurrentGamepad, SDL_GAMEPAD_AXIS_LEFTY);
        int16_t rySDL = SDL_GetGamepadAxis(g_CurrentGamepad, SDL_GAMEPAD_AXIS_RIGHTY);
        pState->Gamepad.sThumbLY = (lySDL == -32768) ? 32767 : -lySDL;
        pState->Gamepad.sThumbRY = (rySDL == -32768) ? 32767 : -rySDL;

        return ERROR_SUCCESS;
    }
    if (oXInputGetState) return oXInputGetState(dwUserIndex, pState);
    return ERROR_DEVICE_NOT_CONNECTED;
}


extern "C" DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
    if (dwUserIndex == 0 && g_CurrentGamepad != nullptr && SDL_GamepadConnected(g_CurrentGamepad))
    {
        SDL_RumbleGamepad(g_CurrentGamepad,
            pVibration->wLeftMotorSpeed,
            pVibration->wRightMotorSpeed,
            5000); // game should cancel before this expires

        return ERROR_SUCCESS;
    }

    if (oXInputSetState) return oXInputSetState(dwUserIndex, pVibration);
    return ERROR_DEVICE_NOT_CONNECTED;
}


extern "C" DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities)
{
    if (dwUserIndex == 0 && g_CurrentGamepad != nullptr && SDL_GamepadConnected(g_CurrentGamepad))
    {
        memset(pCapabilities, 0, sizeof(XINPUT_CAPABILITIES));

        pCapabilities->Type    = XINPUT_DEVTYPE_GAMEPAD;
        pCapabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
        pCapabilities->Flags   = 0;

        pCapabilities->Gamepad.wButtons      = 0xFFFF;
        pCapabilities->Gamepad.bLeftTrigger   = 255;
        pCapabilities->Gamepad.bRightTrigger  = 255;
        pCapabilities->Gamepad.sThumbLX       = 32767;
        pCapabilities->Gamepad.sThumbLY       = 32767;
        pCapabilities->Gamepad.sThumbRX       = 32767;
        pCapabilities->Gamepad.sThumbRY       = 32767;

        pCapabilities->Vibration.wLeftMotorSpeed  = 65535;
        pCapabilities->Vibration.wRightMotorSpeed = 65535;

        return ERROR_SUCCESS;
    }
    if (oXInputGetCapabilities) return oXInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities);
    return ERROR_DEVICE_NOT_CONNECTED;
}


DWORD WINAPI SDLDeviceThread(LPVOID lpParam)
{
    if (!SDL_Init(SDL_INIT_GAMEPAD))
    {
        DEBUG_LOG("SDL3 failed to init: %s", SDL_GetError());
        return 1;
    }

    DEBUG_LOG("SDL3 gamepad initialised");
    SDL_Event event;

    while (true)
    {
        while (SDL_WaitEventTimeout(&event, 10))
        {
            if (event.type == SDL_EVENT_GAMEPAD_ADDED)
            {
                if (g_CurrentGamepad == nullptr)
                {
                    g_CurrentGamepad = SDL_OpenGamepad(event.gdevice.which);
                    if (g_CurrentGamepad)
                    {
                        const char* name = SDL_GetGamepadName(g_CurrentGamepad);
                        DEBUG_LOG("Controller connected: %s", name ? name : "Unknown");

                        //figure out what DI red would be
                        //SDL_SetGamepadLED(g_CurrentGamepad, 0, 200, 180);
                    }
                }
            }
            else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
            {
                SDL_Gamepad* disconnected = SDL_GetGamepadFromID(event.gdevice.which);
                if (disconnected == g_CurrentGamepad)
                {
                    DEBUG_LOG("Gamepad disconnected");
                    SDL_CloseGamepad(g_CurrentGamepad);
                    g_CurrentGamepad = nullptr;
                }
            }
        }
    }
    return 0;
}


void loadAsi() {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("*.asi", &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                printf("Found plugin %s\n", findData.cFileName);
                HMODULE plugin = LoadLibraryA(findData.cFileName);

                if (plugin) {
                    printf("Loaded plugin %s Handle 0x%p\n", findData.cFileName, plugin);
                }
                else {
                    DWORD error = GetLastError();
                    printf("Falied to load plugin %s error %lu\n", findData.cFileName, error);
                }
            }
        }
        while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
}


HRESULT __stdcall hkCreateMasteringVoice(void* pThis, void** ppMasteringVoice,UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, void* pEffectChain)
{
    DEBUG_LOG("CreateMasteringVoice channels=%u rate=%u, forcing 48000Hz", InputChannels, InputSampleRate);
    return oCreateMasteringVoice(pThis, ppMasteringVoice, InputChannels, 48000, Flags, DeviceIndex, pEffectChain);
}

HRESULT WINAPI hkCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv) {
    HRESULT hr = oCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);

    if (SUCCEEDED(hr) && ppv && *ppv &&
        (IsEqualCLSID(rclsid, CLSID_XAudio2) || IsEqualCLSID(rclsid, CLSID_XAudio2_Debug)))
    {
        void** vTable = *reinterpret_cast<void***>(*ppv);

        MH_CreateHook(vTable[10], &hkCreateMasteringVoice, reinterpret_cast<LPVOID*>(&oCreateMasteringVoice));
        MH_EnableHook(vTable[10]);
        DEBUG_LOG("CreateMasteringVoice hooked");
    }
    return hr;
}


HRESULT __stdcall hkEnumDevices(IDirectInput8* pDI, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{//just return straight away this is good since it skips DInput controllers, and keyboard and mouse use CreateDevice with GUID_SysKeyboard/GUID_SysMouse so there fine
    return DI_OK;
}

//needed to skip dinput enum devices
HRESULT WINAPI hkDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    HRESULT hr = oDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

    if (SUCCEEDED(hr) && ppvOut && *ppvOut)
    {
        //steal the vtable
        void** pVtable = *reinterpret_cast<void***>(*ppvOut);
        void* pEnumDevicesTarget = pVtable[4];

        //we only want to create this hook once
        if (oEnumDevices == nullptr)
        {
            MH_CreateHook(pEnumDevicesTarget, &hkEnumDevices, reinterpret_cast<LPVOID*>(&oEnumDevices));
            MH_EnableHook(pEnumDevicesTarget);
        }
    }
    //hand it back to the engine
    return hr;
}


void __fastcall hkSetFOV(void* pThis, void*, float fov) {
    oSetFOV(pThis, fov * Config::FOVMultiplier);
}

//i don't even need this anymore since EnumDevices works fine
bool __cdecl hkIsXInputDevice(const GUID* pGuidProductFromDirectInput) {
    DEBUG_LOG("IsXInputDevice called for %08X", pGuidProductFromDirectInput->Data1);
    return true;
}

bool __fastcall hkCheckZipCrc(void* self) {
    return false;
}

void __fastcall hkCUIFrame(void* pThis_ecx, void* edx, void* pThis_stack)
{
    oCUIFrame(pThis_ecx, pThis_stack);

    //thisa holds the CUIManager pointer
    void* realThis = pThis_stack;

    if (realThis != nullptr)
    {
        float* pMouseSensitivity = (float*)((uintptr_t)realThis + 0xE0);

        int screenY = GetSystemMetrics(SM_CYSCREEN); //calculate the screen Y res
        float screenYfloat = static_cast<float>(screenY); //and turn it to a float

        float multiplier = screenYfloat / 720.0f; //720p should be 1.0f
        *pMouseSensitivity = 1.0f * multiplier;
    }
}


void ExecuteConsoleCommand(const char* cmd) {
    if (pExecuteCommand == nullptr) {
        DEBUG_LOG("pExecuteCommand is nullptr");
    }
    if (pEngineConsole == nullptr) {
        DEBUG_LOG("pEngineConsole is nullptr");
    }
    if (pExecuteCommand && pEngineConsole)
    {
        char buf[256];
        strncpy(buf, cmd, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        pExecuteCommand(pEngineConsole, buf);
    }
}

bool FindEngineConsole(HMODULE hEngine)
{
    const char* consoleAddr = "8B 44 24 04 50 68 ? ? ? ? E8 ? ? ? ? C2 04 00";

    uintptr_t match = Utils::FindPattern(hEngine, consoleAddr);
    if (!match) {
        DEBUG_LOG("Engine console is broken (again)");
    }

    uint8_t* base = reinterpret_cast<uint8_t*>(match);

    pEngineConsole = *reinterpret_cast<void**>(base + 6);

    int32_t relOffset = *reinterpret_cast<int32_t*>(base + 11);
    uintptr_t callTarget = reinterpret_cast<uintptr_t>(base + 15) + relOffset;
    pExecuteCommand = reinterpret_cast<ExecuteCommand_t>(callTarget);

    DEBUG_LOG("Found engine console!");
    DEBUG_LOG("g_EngineConsole = 0x%p", pEngineConsole);
    DEBUG_LOG("ExecuteCommand = 0x%p", pExecuteCommand);
    DEBUG_LOG("IEngineImpl::ExecuteCommand wrapper found at 0x%p", base);

    return true;
}


void __fastcall hkShowLevelAfterFade(void* pThis) {
    oShowLevelAfterFade(pThis); //let the game do its own thing first

    DEBUG_LOG("Loaded level, applying commands");
    if (Config::ForceSOAllShadowMaps) {
        ExecuteConsoleCommand("ForceSOAllShadowMaps 1");
    }

    if (Config::SetScreenshotMode) {
        ExecuteConsoleCommand("SetScreenshotMode 1");
    }
}

DWORD WINAPI ScreenshotThread(LPVOID) {
    while (true)
    {
        if ((GetAsyncKeyState(VK_F5) & 1) && g_IGame && oTakeScreenshot) {
            DEBUG_LOG("IGame::TakeScreenshot called");
            oTakeScreenshot(g_IGame);
        }

        if (GetAsyncKeyState(VK_OEM_PLUS) & 1) {
            Config::FOVMultiplier += 0.1;
            DEBUG_LOG("FOV multiplier is now %.2f", Config::FOVMultiplier);
        }

        if (GetAsyncKeyState(VK_OEM_MINUS) & 1) {
            Config::FOVMultiplier -= 0.1;
            DEBUG_LOG("FOV multiplier is now %.2f", Config::FOVMultiplier);
        }
        Sleep(10);
    }
}


LONG WINAPI hkSetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong) {
    if (hWnd == g_GameHwnd && g_IsWindowed)
    {
        if (nIndex == GWL_STYLE)
        {
            if (dwNewLong == 0x90000000) { //check if the game is fullscreen
                g_IsWindowed = false;
                DEBUG_LOG("Game is fullscreen!");
            }
            else {
                dwNewLong = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
            }
        }
        else if (nIndex ==GWL_EXSTYLE) { //dont think i even need this anymore
            if (dwNewLong & WS_EX_TOPMOST) {
                g_IsWindowed = false;
            }
            else {
                dwNewLong = WS_EX_APPWINDOW;
            }
        }
    }
    return oSetWindowLongA(hWnd, nIndex, dwNewLong);
}

HWND WINAPI hkCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {

    bool isTechlandGame = lpClassName && wcscmp(lpClassName, L"techland_game_class") == 0;
    bool isWindowed = isTechlandGame && (dwExStyle == 0x00040000);

    if (isWindowed) {
        RECT oldClient = { 0, 0, nWidth, nHeight };
        //undo the adjustment
        AdjustWindowRectEx(&oldClient, D3D::kTechlandWindowed_Style, FALSE, dwExStyle);
        int clientW = nWidth - (oldClient.right - oldClient.left - nWidth);
        int clientH = nHeight - (oldClient.bottom - oldClient.top - nHeight);

        dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

        RECT strip = { 0, 0, 0, 0 };
        AdjustWindowRectEx(&strip, D3D::kTechlandWindowed_Style, FALSE, dwExStyle);
        clientW = nWidth  - (strip.right - strip.left);
        clientH = nHeight - (strip.bottom - strip.top);

        //now give it our good style
        RECT rc = { 0, 0, clientW, clientH };
        AdjustWindowRectEx(&rc, D3D::kDesiredStyle, FALSE, dwExStyle);

        nWidth  = rc.right  - rc.left;
        nHeight = rc.bottom - rc.top;

        //let Windows center it instead of using their offset
        X = CW_USEDEFAULT;
        Y = CW_USEDEFAULT;
    }

    HWND hwnd = oCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if (isTechlandGame) {
        g_GameHwnd = hwnd;
        g_IsWindowed = isWindowed;
    }

    return hwnd;
}

//The cpuCount function freaks out when you have a 32 thread CPU, hopefully this should fix it
static void cpuCountFix() {
    DWORD_PTR procMask = 0, sysMask = 0;
    if (!GetProcessAffinityMask(GetCurrentProcess(), &procMask, &sysMask))
        return;

    //only the all bits set 32LP case is broken, bail on anything else
    if (sysMask != 0xFFFFFFFFu)
        return;

    //just drop the top bit, cpuCount will just enumerate 31 LPs and terminate properly
    //We do lose a thread but the game really can only use 4 threads
    SetProcessAffinityMask(GetCurrentProcess(), 0x7FFFFFFFu);
}


DWORD WINAPI MainThread(LPVOID lpParam)
{
    if (!Utils::IsWindows7SP1OrNewer()) {
        MessageBoxW(nullptr, L"BanoiPatch requires Windows 7 SP1 or newer.\n\n" L"The mod will most likely have issues or crash",
            L"BanoiPatch", MB_OK | MB_ICONERROR);

        return 1;
    }

    //Utils::InitialiseConsole();

    Config::Load();
    DEBUG_LOG("FOVMultiplier: %.2f", Config::FOVMultiplier);

    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    strcat_s(sysPath, "\\xinput1_3.dll");

    loadAsi();

    HMODULE hRealXInput = LoadLibraryA(sysPath);
    if (hRealXInput)
    {
        oXInputGetState        = reinterpret_cast<XInputGetState_t>(GetProcAddress(hRealXInput, "XInputGetState"));
        oXInputSetState        = reinterpret_cast<XInputSetState_t>(GetProcAddress(hRealXInput, "XInputSetState"));
        oXInputGetCapabilities = reinterpret_cast<XInputGetCapabilities_t>(GetProcAddress(hRealXInput, "XInputGetCapabilities"));
        DEBUG_LOG("Real xinput1_3.dll loaded from %s", sysPath);
    }

    CreateThread(nullptr, 0, SDLDeviceThread, nullptr, 0, nullptr);

    if (MH_Initialize() != MH_OK)
    {
        DEBUG_LOG("FATAL: MinHook failed to initialise");
        return 1;
    }
    DEBUG_LOG("MinHook initialised");

    HMODULE hEngine = GetModuleHandleW(L"engine_x86_rwdi.dll");
    if (hEngine) {
        uintptr_t pIsXinputDevice = reinterpret_cast<uintptr_t>(hEngine) + 0x1973B0;

        MH_CreateHook(reinterpret_cast<void*>(pIsXinputDevice), &hkIsXInputDevice, reinterpret_cast<LPVOID*>(&oIsXInputDevice));
        MH_EnableHook(reinterpret_cast<void*>(pIsXinputDevice));
        DEBUG_LOG("Hooked IsXinputDevice and removed slow check");

        uintptr_t g_pGameAddr = reinterpret_cast<uintptr_t>(hEngine) + 0x01D9F60C;
        CGame** ppGame = reinterpret_cast<CGame**>(g_pGameAddr);

        while (*ppGame == nullptr) {
            Sleep(1);
        }

        g_Game = *ppGame;
        g_IGame = *(IGame**)((uintptr_t)g_Game + 0x6C);

        DEBUG_LOG("g_pGame %p", g_Game);
        DEBUG_LOG("IGame %p", g_IGame);

        FARPROC pTakeScreenShot = GetProcAddress(hEngine, "?TakeScreenshot@IGame@@QAEXXZ");
        if (pTakeScreenShot) {
            oTakeScreenshot = reinterpret_cast<TakeScreenshot_t>(pTakeScreenShot);
            DEBUG_LOG("IGame::TakeScreenshot hooked %p", pTakeScreenShot);
        }

        FARPROC pSetFOV = GetProcAddress(hEngine, "?SetFOV@IBaseCamera@@QAEXM@Z");
        if (pSetFOV)
        {
            MH_CreateHook(pSetFOV, &hkSetFOV, reinterpret_cast<LPVOID*>(&oSetFOV));
            MH_EnableHook(pSetFOV);
            DEBUG_LOG("SetFOV hooked via export %p", pSetFOV);
        }

        uintptr_t pCUIFrame = reinterpret_cast<uintptr_t>(hEngine) + 0x00358D60;
        if (pCUIFrame) {
            MH_CreateHook(reinterpret_cast<void*>(pCUIFrame), &hkCUIFrame, reinterpret_cast<void**>(&oCUIFrame));
            MH_EnableHook(reinterpret_cast<void*>(pCUIFrame));
            DEBUG_LOG("Hooked CUI::Frame");
        }
        FindEngineConsole(hEngine);
    }


    if (Config::FixWindowed) {
        FARPROC pCreateWindowExW = GetProcAddress(GetModuleHandleW(L"user32.dll"), "CreateWindowExW");
        if (pCreateWindowExW) {
            MH_CreateHook(pCreateWindowExW, &hkCreateWindowExW, reinterpret_cast<void**>(&oCreateWindowExW));
            MH_EnableHook(pCreateWindowExW);
            DEBUG_LOG("Hooked CreateWindowExW");
        }

        FARPROC pSetWindowLongA = GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowLongA");
        if (pSetWindowLongA) {
            MH_CreateHook(pSetWindowLongA, &hkSetWindowLongA, reinterpret_cast<void**>(&oSetWindowLongA));
            MH_EnableHook(pSetWindowLongA);
        }
    }

    HMODULE hOle32 = GetModuleHandleA("ole32.dll");
    if (hOle32) {
        FARPROC pCoCreate = GetProcAddress(hOle32, "CoCreateInstance");
        if (pCoCreate) {
            MH_CreateHook(pCoCreate, &hkCoCreateInstance, reinterpret_cast<LPVOID*>(&oCoCreateInstance));
            MH_EnableHook(pCoCreate);
        }
    }

    HMODULE hDinput8 = LoadLibraryA("dinput8.dll"); //for enumdevices and stuff
    if (hDinput8)
    {
        LPVOID pDirectInput8Create = GetProcAddress(hDinput8, "DirectInput8Create");
        MH_CreateHook(pDirectInput8Create, &hkDirectInput8Create, reinterpret_cast<LPVOID*>(&oDirectInput8Create));
        MH_EnableHook(pDirectInput8Create);
    }

    HWND hwnd = nullptr;
    for (int attempts = 0; attempts < 300 && !hwnd; attempts++) //30 second timeout
    {
        hwnd = FindWindowA("techland_game_class", nullptr);
        if (!hwnd) Sleep(100);
    }

    if (!hwnd)
    {
        DEBUG_LOG("WARNING: Could not find game window, D3D9 hooks skipped");
        return 0;
    }
    DEBUG_LOG("Game window found: 0x%p", hwnd);

    D3D::HookD3D9(hwnd);

    HMODULE hGame = GetModuleHandleW(L"game_x86_rwdi.dll");
    if (hGame)
    {
        uintptr_t baseGame = reinterpret_cast<uintptr_t>(hGame);
        uintptr_t pShowLevel = reinterpret_cast<uintptr_t>(hGame) + 0xD9A60; //swap to using base soon

        MH_CreateHook(reinterpret_cast<void*>(pShowLevel), &hkShowLevelAfterFade,
                       reinterpret_cast<LPVOID*>(&oShowLevelAfterFade));
        MH_EnableHook(reinterpret_cast<void*>(pShowLevel));
        DEBUG_LOG("ShowLevelAfterFade hooked");

        if (Config::enableDevMenu) {
            uint8_t jmpPatch[] = {0xE9, 0xB7, 0x63, 0x1C, 0x00, 0x90};
            Utils::PatchMemory(baseGame + 0x0069E5EF, jmpPatch, sizeof(jmpPatch));

            uint8_t devMenuCave[] = {
                0x8B, 0x82, 0x00, 0x02, 0x00, 0x00,
                0x8B, 0x40, 0x02,
                0x66, 0xC7, 0x40, 0xFF, 0x01, 0x01,
                0x8B, 0x82, 0x00, 0x02, 0x00, 0x00,
                0xE9, 0x30, 0x9C, 0xE3, 0xFF};
            Utils::PatchMemory(baseGame + 0x008649AB, devMenuCave, sizeof(devMenuCave));
            DEBUG_LOG("Enabled dev menu");
        }
    }
    else {
        DEBUG_LOG("Failed to hook ShowLevelAfterFade!");
    }

    CreateThread(nullptr, 0, ScreenshotThread, nullptr, 0, nullptr); //i dont fw creating a whole thread for this

    DEBUG_LOG("BanoiPatch loaded successfully");
    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        cpuCountFix();
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        if (g_CurrentGamepad)
        {
            SDL_CloseGamepad(g_CurrentGamepad);
            g_CurrentGamepad = nullptr;
        }
        SDL_Quit();
    }

    return TRUE;
}