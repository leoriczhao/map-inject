// dllmain.cpp — Entry point for the YDWE-based JAPI DLL
//
// This DLL is loaded via callback exploit (ExportFileFromMpq + LoadLibrary).
// It initializes the YDWE core components adapted for callback mode.

#include <windows.h>
#include <cstdio>
#include <warcraft3/jass/nf_register.h>
#include <warcraft3/jass/hook.h>
#include <warcraft3/war3_searcher.h>

// Forward declarations for lua_engine initialization
namespace warcraft3::lua_engine::lua_loader {
    void initialize();
}

// Forward declarations for yd_jass_api initialization
namespace warcraft3::japi {
    void initialize();
}

static bool g_initialized = false;

extern "C" __declspec(dllexport) void __cdecl Initialize()
{
    if (g_initialized) return;
    g_initialized = true;

    printf("[yd-japi] Initialize() called\n");

    // Step 1: Initialize war3_searcher (version detection)
    // This happens automatically via DO_ONCE in war3_searcher.cpp

    // Step 2: Register JAPI natives and hooks
    // japi_add / japi_hook calls from yd_lua_engine and yd_jass_api
    // have already been queued in add_info_list / hook_info_list.
    // Now trigger the actual registration.

    // Step 3: Initialize the Lua engine
    // This creates the Lua state, registers jass.* modules, loads war3map.lua
    warcraft3::lua_engine::lua_loader::initialize();

    // Step 4: Register yd_jass_api extensions (EX* natives)
    warcraft3::japi::initialize();

    // Step 5: Flush — register all queued japi_add/japi_hook entries
    warcraft3::jass::nf_register::flush();

    printf("[yd-japi] Initialize() complete\n");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        // Don't initialize in DllMain — wait for Initialize() export to be called
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
