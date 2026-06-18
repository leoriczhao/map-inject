// libs_storm.cpp — jass.storm module
//
// Provides storm.load(filename) to read files from MPQ archives via Storm API.
//
// Usage:
//   local storm = require "jass.storm"
//   local content = storm.load("war3map.w3i")
//   if content then print(#content, "bytes") end

#include <lua.hpp>
#include "storm.h"

namespace warcraft3::lua_engine::storm {

	// storm.load(filename) -> string | nil
	// Reads a file from the MPQ archive. Returns file content as a Lua string,
	// or nil if the file is not found.
	static int storm_load(lua_State* L)
	{
		const char* filename = luaL_checkstring(L, 1);

		const void* buf = nullptr;
		size_t len = 0;
		if (!storm_s::instance().load_file(filename, &buf, &len)) {
			lua_pushnil(L);
			return 1;
		}

		lua_pushlstring(L, (const char*)buf, len);
		storm_s::instance().unload_file(buf);
		return 1;
	}

	// storm.has(filename) -> boolean
	// Checks if a file exists in the MPQ archive.
	static int storm_has(lua_State* L)
	{
		const char* filename = luaL_checkstring(L, 1);
		lua_pushboolean(L, storm_s::instance().has_file(filename));
		return 1;
	}

	int open(lua_State* L)
	{
		lua_newtable(L);

		lua_pushstring(L, "load");
		lua_pushcfunction(L, storm_load);
		lua_rawset(L, -3);

		lua_pushstring(L, "has");
		lua_pushcfunction(L, storm_has);
		lua_rawset(L, -3);

		return 1;
	}
}
