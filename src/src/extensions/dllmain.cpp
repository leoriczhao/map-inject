// dllmain.cpp — Entry point for the YDWE-based JAPI DLL
//
// This DLL is loaded via callback exploit (ExportFileFromMpq + LoadLibrary).
// It initializes the YDWE core components adapted for callback mode.

#include <windows.h>
#include <cstdio>
#include <warcraft3/jass.h>
#include <warcraft3/jass/nf_register.h>
#include <warcraft3/jass/hook.h>
#include <warcraft3/war3_searcher.h>
#include "../lua_engine/lua_engine/lua_loader.h"
#include "../lua_engine/lua_engine/bridge_dispatch.h"
#include "debug_log.h"

// Forward declarations
namespace warcraft3::lua_engine::lua_loader {
    void initialize();
    void load_script();
}
namespace warcraft3::japi {
    void initialize();
    void initialize_ex_natives();
    uint32_t open_code_run_logs_handler(const uint32_t* args, size_t nargs);
}
namespace warcraft3::lua_engine::bridge {
    typedef uint32_t (*cpp_handler_fn)(const uint32_t* args, size_t nargs);
    void register_cpp_handler(const char* name, const char* spec, cpp_handler_fn fn);
    void initialize();
}

static bool g_initialized = false;

// HookUnitId — registered as JASS native via japi_add.
// Called from war3map.j AFTER callback returns, when hash table is available.
// Hooks UnitId via table_hook, then loads war3map.lua.
extern "C" __declspec(dllexport) uint32_t __cdecl HookUnitId()
{
    static bool done = false;
    if (done) return 0;
    done = true;

    DBG_LOG("HookUnitId() called — hooking UnitId");
    warcraft3::lua_engine::bridge::initialize();
    DBG_LOG("HookUnitId() — UnitId hooked, loading script");
    warcraft3::lua_engine::lua_loader::load_script();
    DBG_LOG("HookUnitId() done");
    return 0;
}

// Synchronous debug log macro (works even if async log hasn't flushed)
#define SYNC_LOG(msg) do { \
    HANDLE _h = CreateFileA("C:\\ProgramData\\japi_sync.log", \
        FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, \
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); \
    if (_h != INVALID_HANDLE_VALUE) { \
        DWORD _bw; const char* _m = msg "\r\n"; \
        WriteFile(_h, _m, (DWORD)strlen(_m), &_bw, NULL); \
        CloseHandle(_h); \
    } \
} while(0)

extern "C" __declspec(dllexport) void __cdecl Initialize()
{
    if (g_initialized) return;
    g_initialized = true;

    SYNC_LOG("Initialize() START");

    // Step 1: Create Lua VM (no JASS calls, no script loading)
    SYNC_LOG("Step 1: lua_loader START");
    warcraft3::lua_engine::lua_loader::initialize();
    SYNC_LOG("Step 1: lua_loader DONE");

    // Step 2: Register EX* natives via bridge dispatch
    SYNC_LOG("Step 2: initialize_ex_natives START");
    warcraft3::japi::initialize_ex_natives();
    SYNC_LOG("Step 2: initialize_ex_natives DONE");

    // Step 3: Register bridge-dispatched handlers (UnitId hashtable RPC)
    warcraft3::lua_engine::bridge::register_cpp_handler(
        "open_code_run_logs", "(B)V",
        warcraft3::japi::open_code_run_logs_handler);

    // Step 3: Flush queued natives (add only, no hash table modification)
    warcraft3::jass::nf_register::flush_add_only();
    SYNC_LOG("Step 3: flush_add_only DONE");

    SYNC_LOG("Initialize() DONE");
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
