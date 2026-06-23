// bridge_dispatch.cpp — UnitId-based JASS→Lua bridge dispatch
//
// The built-in JAPI pattern: JASS calls UnitId("FuncName"), the DLL intercepts,
// reads args from a shared hashtable, dispatches to a handler, writes result back.
// This keeps war3map.j clean (only standard JASS natives).

#include "bridge_dispatch.h"
#include "lua_loader.h"
#include "callback.h"
#include "jassbind.h"
#include "libs_runtime.h"
#include <warcraft3/jass.h>
#include <warcraft3/jass/hook.h>
#include <warcraft3/jass/func_value.h>
#include <base/hook/fp_call.h>
#include <lua.hpp>
#include <map>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "debug_log.h"

namespace warcraft3::lua_engine::bridge {

    // Log to both debug log and in-game screen
    static void log_message(const char* msg) {
        DBG_LOG("%s", msg);
        jass::string_fake sf(msg);
        jass::call("DisplayTimedTextToPlayer",
            jass::call("GetLocalPlayer"), 0, 0, 10.0f, (jass::jstring_t)sf);
    }

    // ── State ──────────────────────────────────────────────────────

    static uintptr_t RealUnitId = 0;
    static uint32_t  g_ht_handle = 0;   // JASS hashtable handle
    static uint32_t  g_ht_key = 0;      // StringHash("jass")

    struct handler_entry {
        std::string    spec;       // e.g. "(II)I"
        int            lua_ref;    // Lua registry reference (0 for C++ handlers)
        cpp_handler_fn cpp_fn;    // C++ handler (nullptr for Lua handlers)
    };

    static std::map<std::string, handler_entry> g_handlers;

    // ── Spec parser ────────────────────────────────────────────────

    struct param_info {
        char type;  // 'I', 'R', 'S', 'B', 'H'
    };

    struct parsed_spec {
        std::vector<param_info> params;
        char                    ret;  // 'I', 'R', 'S', 'B', 'V'
    };

    static parsed_spec parse_spec(const char* spec) {
        parsed_spec result;
        result.ret = 'V';
        if (!spec || spec[0] != '(') return result;

        const char* p = spec + 1;
        while (*p && *p != ')') {
            if (*p == ';') { p++; continue; }  // skip semicolons
            param_info pi;
            pi.type = *p;
            result.params.push_back(pi);
            p++;
            // skip lowercase subtype tags (e.g. Hplayer;)
            while (*p && *p >= 'a' && *p <= 'z') p++;
        }
        if (*p == ')') p++;
        if (*p) result.ret = *p;
        return result;
    }

    // ── Hashtable access via JASS natives ──────────────────────────
    // We call the game's Save/Load natives directly through the JASS VM.

    static void ht_save_str(uint32_t ht, uint32_t key, int slot, const char* value) {
        jass::string_fake sv(value);
        jass::call("SaveStr", ht, key, slot, (jass::jstring_t)sv);
    }

    static void ht_save_int(uint32_t ht, uint32_t key, int slot, int32_t value) {
        jass::call("SaveInteger", ht, key, slot, value);
    }

    static void ht_save_real(uint32_t ht, uint32_t key, int slot, uint32_t value) {
        jass::call("SaveReal", ht, key, slot, value);
    }

    static void ht_save_bool(uint32_t ht, uint32_t key, int slot, uint32_t value) {
        jass::call("SaveBoolean", ht, key, slot, value);
    }

    static const char* ht_load_str(uint32_t ht, uint32_t key, int slot) {
        jass::jstring_t result = (jass::jstring_t)jass::call("LoadStr", ht, key, slot);
        return jass::from_string(result);
    }

    static int32_t ht_load_int(uint32_t ht, uint32_t key, int slot) {
        return (int32_t)jass::call("LoadInteger", ht, key, slot);
    }

    static uint32_t ht_load_real(uint32_t ht, uint32_t key, int slot) {
        return (uint32_t)jass::call("LoadReal", ht, key, slot);
    }

    static uint32_t ht_load_bool(uint32_t ht, uint32_t key, int slot) {
        return (uint32_t)jass::call("LoadBoolean", ht, key, slot);
    }

    // ── Handler dispatch ───────────────────────────────────────────

    static void dispatch_handler(const char* name, const handler_entry& h) {
        parsed_spec spec = parse_spec(h.spec.c_str());

        // C++ handler: read args from ht, call, write result back
        if (h.cpp_fn) {
            uint32_t args[16] = {0};
            for (size_t i = 0; i < spec.params.size() && i < 16; i++) {
                int slot = (int)i + 1;
                switch (spec.params[i].type) {
                    case 'I': case 'H':
                        args[i] = (uint32_t)ht_load_int(g_ht_handle, g_ht_key, slot);
                        break;
                    case 'R':
                        args[i] = ht_load_real(g_ht_handle, g_ht_key, slot);
                        break;
                    case 'B':
                        args[i] = ht_load_bool(g_ht_handle, g_ht_key, slot);
                        break;
                    default:
                        args[i] = (uint32_t)ht_load_int(g_ht_handle, g_ht_key, slot);
                        break;
                }
            }
            // Strings need special handling — pass the JASS string handle
            // For now, handle 'S' by loading as integer (jstring_t is uint32_t)
            // The C++ handler must call jass::from_string() to get the actual string
            uint32_t result = h.cpp_fn(args, spec.params.size());

            if (spec.ret != 'V') {
                switch (spec.ret) {
                    case 'I': case 'H':
                        ht_save_int(g_ht_handle, g_ht_key, 0, (int32_t)result);
                        break;
                    case 'R':
                        ht_save_real(g_ht_handle, g_ht_key, 0, result);
                        break;
                    case 'S':
                        // result is a jstring_t handle
                        ht_save_str(g_ht_handle, g_ht_key, 0,
                            jass::from_string((jass::jstring_t)result));
                        break;
                    case 'B':
                        ht_save_bool(g_ht_handle, g_ht_key, 0, result ? 1 : 0);
                        break;
                }
            }
            return;
        }

        lua_State* L = lua_loader::getMainL();
        if (!L) return;

        // Push the Lua function from registry
        lua_rawgeti(L, LUA_REGISTRYINDEX, h.lua_ref);

        // Read args from hashtable and push to Lua stack
        for (size_t i = 0; i < spec.params.size(); i++) {
            int slot = (int)i + 1;
            switch (spec.params[i].type) {
                case 'I':
                case 'H':
                    lua_pushinteger(L, ht_load_int(g_ht_handle, g_ht_key, slot));
                    break;
                case 'R': {
                    uint32_t rv = ht_load_real(g_ht_handle, g_ht_key, slot);
                    jassbind::push_real(L, rv);
                    break;
                }
                case 'S': {
                    const char* s = ht_load_str(g_ht_handle, g_ht_key, slot);
                    lua_pushstring(L, s ? s : "");
                    break;
                }
                case 'B':
                    lua_pushboolean(L, ht_load_bool(g_ht_handle, g_ht_key, slot));
                    break;
                default:
                    lua_pushinteger(L, ht_load_int(g_ht_handle, g_ht_key, slot));
                    break;
            }
        }

        // Call the Lua function
        int error = safe_call(L, (int)spec.params.size(),
                              (spec.ret != 'V') ? 1 : 0, true);

        // Write result back to hashtable
        if (error == LUA_OK && spec.ret != 'V') {
            switch (spec.ret) {
                case 'I':
                case 'H': {
                    int32_t val = (int32_t)lua_tointeger(L, -1);
                    ht_save_int(g_ht_handle, g_ht_key, 0, val);
                    break;
                }
                case 'R': {
                    jass::jreal_t rv = jassbind::read_real(L, -1);
                    ht_save_real(g_ht_handle, g_ht_key, 0, rv);
                    break;
                }
                case 'S': {
                    const char* s = lua_tostring(L, -1);
                    ht_save_str(g_ht_handle, g_ht_key, 0, s ? s : "");
                    break;
                }
                case 'B': {
                    uint32_t bv = lua_toboolean(L, -1) ? 1 : 0;
                    ht_save_bool(g_ht_handle, g_ht_key, 0, bv);
                    break;
                }
            }
            lua_pop(L, 1);
        }
    }

    // ── UnitId hook ────────────────────────────────────────────────

    static jass::jstring_t __cdecl FakeUnitId(jass::jstring_t name_str) {
        const char* name = jass::from_string(name_str);
        if (!name) {
            return base::c_call<jass::jstring_t>(RealUnitId, name_str);
        }

        // Initialization: UnitId(I2S(GetHandleId(japi_ht)))
        // The string is a pure number — store as hashtable handle
        if (name[0] >= '0' && name[0] <= '9') {
            g_ht_handle = (uint32_t)std::atoll(name);
            g_ht_key = (uint32_t)jass::call("StringHash", jass::string_fake("jass"));
            log_message("[bridge] ht initialized");
            return 0;
        }

        // Dispatch: UnitId("FuncName")
        auto it = g_handlers.find(name);
        if (it != g_handlers.end()) {
            dispatch_handler(name, it->second);
            return 0;
        }

        // Unknown — pass through to original UnitId
        return base::c_call<jass::jstring_t>(RealUnitId, name_str);
    }

    // ── Public API ─────────────────────────────────────────────────

    void register_handler(lua_State* L, const char* name, const char* spec, int func_index) {
        handler_entry entry;
        entry.spec = spec;
        entry.cpp_fn = nullptr;
        // Store a reference to the Lua function
        lua_pushvalue(L, func_index);
        entry.lua_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        g_handlers[name] = entry;
        DBG_LOG("[bridge] registered handler: %s %s", name, spec);
    }

    void register_cpp_handler(const char* name, const char* spec, cpp_handler_fn fn) {
        handler_entry entry;
        entry.spec = spec;
        entry.lua_ref = 0;
        entry.cpp_fn = fn;
        g_handlers[name] = entry;
        DBG_LOG("[bridge] registered C++ handler: %s %s", name, spec);
    }

    void initialize() {
        // Hook UnitId via table_hook.
        // NOTE: Do NOT call log_message() here — it calls JASS functions
        // which may crash during callback context.
        jass::table_hook("UnitId", (uintptr_t*)&RealUnitId, (uintptr_t)FakeUnitId);
    }

    uint32_t get_ht_handle() { return g_ht_handle; }
    uint32_t get_ht_key() { return g_ht_key; }

}  // namespace warcraft3::lua_engine::bridge
