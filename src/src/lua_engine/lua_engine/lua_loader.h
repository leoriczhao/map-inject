#pragma once

#include <cstdint>
#include <warcraft3/jass.h>

struct lua_State;

namespace warcraft3::lua_engine::lua_loader {
	void initialize();
	lua_State* getMainL();
}
