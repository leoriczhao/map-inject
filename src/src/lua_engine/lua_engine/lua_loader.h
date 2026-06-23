#pragma once

#include <cstdint>
#include <warcraft3/jass.h>

struct lua_State;

namespace warcraft3::lua_engine::lua_loader {
	void initialize();
	lua_State* getMainL();
	void load_script();
	uint32_t LoadScript_bridge(const uint32_t* args, size_t nargs);
}
