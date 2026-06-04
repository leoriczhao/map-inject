// bee/lua/binding.h — compatibility shim
#pragma once

#include <lua.hpp>
#include <stdexcept>

namespace bee::lua {
    inline int push_error(lua_State* L, const std::exception& e) {
        lua_pushstring(L, e.what());
        return lua_error(L);
    }
}
