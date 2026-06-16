// extensions/custom_natives.cpp — User-defined JAPI extension natives
//
// Register custom C++ functions as JASS-callable natives here.
// These are accessible from Lua via jass.XXX() or from JASS directly.
//
// Registration pattern (in initialize()):
//   jass::japi_add((uintptr_t)my_func, "MyFuncName", "(II)I");
//
// Parameter spec format:
//   I = integer, R = real, S = string, H = handle, V = void (return)
//   Example: "(II)I" = two ints -> int, "(IS)V" = int+string -> void

#include <warcraft3/jass.h>
#include <warcraft3/jass/hook.h>
#include <warcraft3/war3_searcher.h>
#include <cstdint>

namespace warcraft3::japi {

// === Example: simple integer function ===
// Uncomment and modify to add your own natives.
//
// uint32_t __cdecl EXGetUnitMaxLife(uint32_t unit_handle)
// {
//     // Example: read unit max life via memory offset
//     // Real implementation would use handle_to_object + offset
//     return 0;
// }
//
// void InitializeCustomUnitState()
// {
//     jass::japi_add((uintptr_t)EXGetUnitMaxLife, "EXGetUnitMaxLife", "(Hunit;)");
// }

void initialize()
{
    // Register all custom extension natives here.
    // Each Initialize* function calls jass::japi_add() to queue a native.
    // The actual registration happens later in nf_register::flush().
    //
    // InitializeCustomUnitState();
}

}  // namespace warcraft3::japi
