#pragma once
 
#include <windows.h>
#include <d3d9.h>
#include <dinput.h>
 
#ifndef XINPUT_GAMEPAD_DPAD_UP
 
#define XINPUT_GAMEPAD_DPAD_UP        0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN      0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT      0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT     0x0008
#define XINPUT_GAMEPAD_START          0x0010
#define XINPUT_GAMEPAD_BACK           0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB     0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB    0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER  0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define XINPUT_GAMEPAD_A              0x1000
#define XINPUT_GAMEPAD_B              0x2000
#define XINPUT_GAMEPAD_X              0x4000
#define XINPUT_GAMEPAD_Y              0x8000
 
#define XINPUT_DEVTYPE_GAMEPAD        0x01
#define XINPUT_DEVSUBTYPE_GAMEPAD     0x01
 
typedef struct _XINPUT_GAMEPAD {
    WORD  wButtons;
    BYTE  bLeftTrigger;
    BYTE  bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} XINPUT_GAMEPAD;
 
typedef struct _XINPUT_STATE {
    DWORD          dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE;
 
typedef struct _XINPUT_VIBRATION {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} XINPUT_VIBRATION;
 
typedef struct _XINPUT_CAPABILITIES {
    BYTE             Type;
    BYTE             SubType;
    WORD             Flags;
    XINPUT_GAMEPAD   Gamepad;
    XINPUT_VIBRATION Vibration;
} XINPUT_CAPABILITIES;
 
#endif // XINPUT_GAMEPAD_DPAD_UP

class IGame;
class CGSObject;
class CGame;
 
typedef DWORD(WINAPI* XInputGetState_t)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI* XInputSetState_t)(DWORD, XINPUT_VIBRATION*);
typedef DWORD(WINAPI* XInputGetCapabilities_t)(DWORD, DWORD, XINPUT_CAPABILITIES*);
 
typedef HRESULT(__stdcall* SetSamplerState_t)(IDirect3DDevice9*, DWORD, D3DSAMPLERSTATETYPE, DWORD);
 
typedef HRESULT(WINAPI* DirectInput8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(__stdcall* EnumDevices_t)(IDirectInput8A*, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
 
typedef HRESULT(WINAPI* XAudio2Create_t)(void** ppXAudio2, UINT32 Flags, UINT32 XAudio2Processor);
typedef HRESULT(__stdcall* CreateMasteringVoice_t)(void* pThis, void** ppMasteringVoice, UINT32 InputChannels,UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, void* pEffectChain);
typedef HRESULT(WINAPI* CoCreateInstance_t)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);

typedef BOOL (__cdecl* IsXInputDevice_t)(const GUID*);
typedef void(__thiscall* SetFOV_t)(void* pThis, float fov);
typedef void(__thiscall* ShowLevelAfterFade_t)(void* pThis);
typedef bool(__stdcall* ExecuteCommand_t)(void* pEngineConsole, const char* pszCommand);
using TakeScreenshot_t = void(__thiscall*)(void*);
using StartAVIRecording_t = void(__thiscall*)(void*);

typedef HRESULT(WINAPI* DirectInput8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
typedef HRESULT(__stdcall* EnumDevices_t)(IDirectInput8*, DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD);
typedef void (__cdecl *EnableCommentRender_t)(bool);

using CheckCrcOpenedFile_t = bool(__thiscall*)(void* pThis, int* param_1, unsigned char** param_2);

typedef void(__cdecl* SetPlatform_t)(const char* platform_name);

using SetWindowLongA_t = LONG(WINAPI*)(HWND hWnd, int nIndex, LONG dwNewLong);

using CreateWindowExW_t = HWND(WINAPI*)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName,DWORD dwStyle, int X, int Y, int nWidth, int nHeight,HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

typedef bool (__thiscall* tCheckZipCRC)(void* self);
typedef void(__thiscall* tCUIFrame)(void* pThis_ecx, void* pThis_stack);
