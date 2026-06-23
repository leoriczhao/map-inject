// bridge_dispatch.h — UnitId-based JASS→Lua bridge dispatch
//
// Hooks the JASS native UnitId() to intercept calls like UnitId("GetMouseX").
// The DLL reads the spec and arguments from a shared hashtable,
// dispatches to the registered handler (C++ or Lua), and writes the result back.

#pragma once

#include <cstdint>
#include <lua.hpp>
#include <warcraft3/jass.h>

namespace warcraft3::lua_engine::bridge {

    // C++ handler function type: receives parsed args as array,
    // returns result. The bridge dispatch reads/writes the hashtable.
    // args[0..nargs-1] are the parameters, return value goes to ht slot 0.
    typedef uint32_t (*cpp_handler_fn)(const uint32_t* args, size_t nargs);

    // Register a Lua function as a JAPI handler.
    // Called from libs_japi.cpp when Lua does japi("Name", "(II)I", func).
    void register_handler(lua_State* L, const char* name, const char* spec, int func_index);

    // Register a C++ function as a JAPI handler.
    // Called from lua_loader.cpp for built-in functions like EXExecuteScript.
    void register_cpp_handler(const char* name, const char* spec, cpp_handler_fn fn);

    // Initialize the UnitId hook. Called from lua_loader::initialize().
    void initialize();

    // Set the real UnitId function pointer (for external hooking).
    void set_real_unitid(uintptr_t addr);

    // Get the FakeUnitId function pointer (for external hooking).
    uintptr_t get_fake_unitid();

    // Get the shared hashtable handle (set during JASS init via UnitId(I2S(GetHandleId(japi_ht)))).
    uint32_t get_ht_handle();

    // Get the hashtable key (StringHash("jass")).
    uint32_t get_ht_key();

}  // namespace warcraft3::lua_engine::bridge
