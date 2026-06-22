// dllmain.cpp — Entry point for the YDWE-based JAPI DLL
//
// This DLL is loaded via callback exploit (ExportFileFromMpq + LoadLibrary).
// It initializes the YDWE core components adapted for callback mode.

#include <windows.h>
#include <cstdio>
#include <warcraft3/jass/nf_register.h>
#include <warcraft3/jass/hook.h>
#include <warcraft3/war3_searcher.h>
#include "debug_log.h"

// Forward declarations for lua_engine initialization
namespace warcraft3::lua_engine::lua_loader {
    void initialize();
}

// Forward declarations for yd_jass_api initialization
namespace warcraft3::japi {
    void initialize();
}

// Forward declarations for bridge dispatch registration
namespace warcraft3::lua_engine::bridge {
    typedef uint32_t (*cpp_handler_fn)(const uint32_t* args, size_t nargs);
    void register_cpp_handler(const char* name, const char* spec, cpp_handler_fn fn);
}

static bool g_initialized = false;

extern "C" __declspec(dllexport) void __cdecl Initialize()
{
    if (g_initialized) return;
    g_initialized = true;

    DBG_LOG("Initialize() START");

    // Initialize the Lua engine
    // This creates the Lua state, registers jass.* modules, loads war3map.lua
    warcraft3::lua_engine::lua_loader::initialize();

    DBG_LOG("lua_loader::initialize() DONE");

    // Register yd_jass_api extensions (EX* natives)
    warcraft3::japi::initialize();

    // Flush — register all queued japi_add/japi_hook entries
    warcraft3::jass::nf_register::flush();

    // Initialize the D3D9 proxy API (newD3d9.dll)
    // This registers all d3d.* handlers (CreateFont, CreateText, etc.)
    // with the bridge dispatch so JASS can call them via UnitId.
    {
        HMODULE hD3D9 = GetModuleHandleA("newD3d9.dll");
        if (!hD3D9) hD3D9 = GetModuleHandleA("d3d9.dll");
        if (hD3D9) {
            // InitializeD3DAPI(register_handler_fn, from_string_fn)
            // register_handler_fn: void(*)(const char*, const char*, handler_fn)
            // from_string_fn: const char*(*)(uint32_t)
            typedef void (__cdecl *InitD3DAPIFn)(
                void(*)(const char*, const char*, uint32_t(*)(const uint32_t*, size_t)),
                const char*(*)(uint32_t));
            auto init_fn = reinterpret_cast<InitD3DAPIFn>(
                GetProcAddress(hD3D9, "InitializeD3DAPI"));
            if (init_fn) {
                init_fn(
                    warcraft3::lua_engine::bridge::register_cpp_handler,
                    warcraft3::jass::from_string);
                DBG_LOG("InitializeD3DAPI() called successfully");
            } else {
                DBG_LOG("InitializeD3DAPI not found in D3D9 module");
            }
        } else {
            DBG_LOG("newD3d9.dll not loaded yet — d3d.* handlers not registered");
        }
    }

    DBG_LOG("Initialize() COMPLETE");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        DBG_INIT();
        DBG_LOG("DllMain ATTACH module=%p", hModule);
        break;
    case DLL_PROCESS_DETACH:
        DBG_SHUTDOWN();
        break;
    }
    return TRUE;
}
