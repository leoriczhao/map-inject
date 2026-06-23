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
#include <warcraft3/hashtable.h>
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

// HookUnitId — exported for completeness but not called from war3map.j anymore.
extern "C" __declspec(dllexport) uint32_t __cdecl HookUnitId()
{
    return 0;
}

// Synchronous debug log macro
inline void sync_log_impl(const char* msg) {
    HANDLE h = CreateFileA("C:\\ProgramData\\japi_sync.log",
        FILE_APPEND_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD bw;
        WriteFile(h, msg, (DWORD)strlen(msg), &bw, NULL);
        WriteFile(h, "\r\n", 2, &bw, NULL);
        CloseHandle(h);
    }
}
#define SYNC_LOG(msg) sync_log_impl(msg)

// Helper: find a native in the linked list by matching its hash table func_address.
// Returns the address of the func_address field in the linked list node, or 0.
// Linked list node layout: [+0] = next, [+12] = func_address
// Linked list head: [[GameDLL + 0xBE3740] + 0x14] + 0x20
static uintptr_t find_linked_list_func_addr(const char* name)
{
    uintptr_t game_base = warcraft3::get_war3_searcher().base();
    uintptr_t jass_env = *(uintptr_t*)(game_base + 0xBE3740);
    if (!jass_env) return 0;

    uintptr_t ptr1 = *(uintptr_t*)(jass_env + 0x14);
    if (!ptr1) return 0;

    uintptr_t first_node = *(uintptr_t*)(ptr1 + 0x20);
    if (!first_node) return 0;

    auto* ht_node = warcraft3::get_native_function_hashtable()->find(name);
    if (!ht_node) return 0;

    uint32_t target = ht_node->func_address_;
    for (uintptr_t cur = first_node; cur && cur > 0x10000; cur = *(uintptr_t*)cur) {
        if (*(uint32_t*)(cur + 12) == target) {
            return cur + 12;  // address of func_address field
        }
        if (*(uintptr_t*)cur == first_node) break;
    }
    return 0;
}

extern "C" __declspec(dllexport) void __cdecl Initialize()
{
    if (g_initialized) return;
    g_initialized = true;

    SYNC_LOG("Initialize() START");

    // Step 1: Create Lua VM
    warcraft3::lua_engine::lua_loader::initialize();
    SYNC_LOG("Step 1: lua_loader DONE");

    // Step 2: Register EX* natives
    warcraft3::japi::initialize_ex_natives();
    SYNC_LOG("Step 2: initialize_ex_natives DONE");

    // Step 3: Register bridge handlers + flush natives
    warcraft3::lua_engine::bridge::register_cpp_handler(
        "open_code_run_logs", "(B)V",
        warcraft3::japi::open_code_run_logs_handler);
    warcraft3::jass::nf_register::flush_add_only();
    SYNC_LOG("Step 3: flush_add_only DONE");

    // Step 4: Hook UnitId via linked list (same as callback's CreateJassNativeHook)
    {
        uintptr_t addr = find_linked_list_func_addr("UnitId");
        if (addr) {
            uint32_t orig = *(uint32_t*)addr;
            warcraft3::lua_engine::bridge::set_real_unitid(orig);
            *(uint32_t*)addr = (uint32_t)warcraft3::lua_engine::bridge::get_fake_unitid();
            char buf[128];
            snprintf(buf, sizeof(buf), "Step 4: UnitId hooked @ %p, orig=0x%08X", (void*)addr, orig);
            SYNC_LOG(buf);
        } else {
            SYNC_LOG("Step 4: UnitId NOT FOUND in linked list");
        }
    }

    // Step 5: Register LoadScript bridge handler
    // war3map.j calls SaveStr(ht,key,0,"()V") then UnitId("LoadScript")
    warcraft3::lua_engine::bridge::register_cpp_handler(
        "LoadScript", "()V",
        [](const uint32_t*, size_t) -> uint32_t {
            SYNC_LOG("LoadScript: loading war3map.lua");
            warcraft3::lua_engine::lua_loader::load_script();
            SYNC_LOG("LoadScript: done");
            return 0;
        });
    SYNC_LOG("Step 5: LoadScript handler registered");

    SYNC_LOG("Initialize() DONE");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        DBG_INIT();
        break;
    case DLL_PROCESS_DETACH:
        DBG_SHUTDOWN();
        break;
    }
    return TRUE;
}
