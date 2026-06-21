#include "src/D3DHooks.h"

namespace D3D {

    SetSamplerState_t oSetSamplerState = nullptr;

    HRESULT __stdcall hkSetSamplerState(IDirect3DDevice9* pDevice, DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) //force 16x af
    {
        if (Type == D3DSAMP_MAGFILTER || Type == D3DSAMP_MINFILTER || Type == D3DSAMP_MIPFILTER)
        {
            Value = D3DTEXF_ANISOTROPIC;
            oSetSamplerState(pDevice, Sampler, D3DSAMP_MAXANISOTROPY, 16);
        }

        if (Type == D3DSAMP_MAXANISOTROPY) //annoying I have to do this as well
        {
            Value = 16;
        }

        return oSetSamplerState(pDevice, Sampler, Type, Value);
    }

    void HookD3D9(HWND hwnd)
    {
        IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D)
        {
            DEBUG_LOG("Failed to create D3D9 for vtable steal");
            return;
        }

        D3DPRESENT_PARAMETERS d3dpp = {};
        d3dpp.Windowed       = TRUE;
        d3dpp.SwapEffect     = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow  = hwnd;

        IDirect3DDevice9* pDummy = nullptr;
        HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummy);

        if (SUCCEEDED(hr))
        {
            void** vtable = *reinterpret_cast<void***>(pDummy);

            //IDirect3DDevice9::SetSamplerState is vtable index 69
            void* pSetSamplerState = vtable[69];

            MH_CreateHook(pSetSamplerState, &hkSetSamplerState, reinterpret_cast<LPVOID*>(&oSetSamplerState));
            MH_EnableHook(pSetSamplerState);
            DEBUG_LOG("SetSamplerState hooked, 16x AF enabled");


            pDummy->Release();
        }
        else{
            DEBUG_LOG("Failed to create dummy D3D9 device (HRESULT: 0x%08X)", hr);
        }
        pD3D->Release();
    }
}