#pragma once

struct lua_State;

namespace warcraft3::lua_engine {
	lua_State* newstate();
	int fix_baselib(lua_State* L);
}
