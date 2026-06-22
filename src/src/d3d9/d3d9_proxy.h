// d3d9_proxy.h — Forward declarations for D3D9 proxy classes
//
// newD3d9.dll is a D3D9 proxy that War3 loads instead of the real d3d9.dll.
// It wraps IDirect3D9 and IDirect3DDevice9, hooking EndScene for custom
// rendering and Reset for device-lost handling.

#pragma once

#include <windows.h>
#include <d3d9.h>

// Forward declarations
class CDirect3D9;
class CDirect3DDevice9;

// Global state
namespace d3d9_proxy {
    // The real d3d9.dll module handle
    HMODULE get_real_d3d9_module();

    // Real Direct3DCreate9 function pointer
    typedef IDirect3D9* (WINAPI *Direct3DCreate9_fn)(UINT SDKVersion);

    // Get the War3 window handle (for screen size queries, mouse position, etc.)
    HWND get_war3_hwnd();
}
