// fix_baselib.cpp — Simplified for callback mode

#include "fix_baselib.h"
#include <lua.hpp>

namespace warcraft3::lua_engine {

	lua_State* newstate()
	{
		return luaL_newstate();
	}

	int fix_baselib(lua_State* L)
	{
		(void)L;
		return 0;
	}
}
