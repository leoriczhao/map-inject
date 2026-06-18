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
    void initialize_ex_natives();
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

    // Register EX* natives ported from YDWE yd_jass_api
    warcraft3::japi::initialize_ex_natives();

    // Flush — register all queued japi_add/japi_hook entries
    warcraft3::jass::nf_register::flush();

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
