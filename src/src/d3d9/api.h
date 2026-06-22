// api.h — D3D9 API handler registration for UnitId dispatch
//
// Registers C++ handler functions for all d3d.* JASS API calls.
// These handlers read arguments from the JASS hashtable and call
// into the renderer to create/manage fonts, text, and textures.
//
// Called from yd_japi.dll via exported InitializeD3DAPI() function.

#pragma once

#include <cstdint>
#include <cstddef>

namespace d3d9_api {

    // Handler function type (matches bridge::cpp_handler_fn)
    typedef uint32_t (*handler_fn)(const uint32_t* args, size_t nargs);

    // Registration function type (matches bridge::register_cpp_handler)
    typedef void (*register_handler_fn)(const char* name, const char* spec, handler_fn fn);

    // Register all d3d API handlers with the bridge dispatch.
    // Called once from InitializeD3DAPI().
    void register_all_handlers(register_handler_fn reg_fn);

    // Set the jass::from_string function pointer for string arg conversion.
    typedef const char* (*from_string_fn)(uint32_t);
    void set_from_string_fn(from_string_fn fn);

}  // namespace d3d9_api
