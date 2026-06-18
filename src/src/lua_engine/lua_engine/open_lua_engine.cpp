// open_lua_engine.cpp — Simplified version for callback mode
// Registers only the essential jass.* Lua modules.

#include "open_lua_engine.h"
#include "callback.h"
#include "class_real.h"
#include "class_array.h"
#include "class_handle.h"

namespace warcraft3::lua_engine {

	namespace common { int open(lua_State* L); }
	namespace globals { int open(lua_State* L); }
	namespace japi { int open(lua_State* L); }
	namespace hook { int open(lua_State* L); }
	namespace runtime { int open(lua_State* L); }
	namespace debug { int open(lua_State* L); }
	namespace message { int open(lua_State* L); }
	namespace sleep { int open(lua_State* L); }
	namespace storm { int open(lua_State* L); }
	namespace slk { int open(lua_State* L); }

	void register_preload_lib(lua_State* L, const char *name, lua_CFunction f)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
		lua_pushcclosure(L, f, 0);
		lua_setfield(L, -2, name);
		lua_pop(L, 1);
	}

	int open_lua_engine(lua_State* L)
	{
		// Core modules
		register_preload_lib(L, "jass.common",  common::open);
		register_preload_lib(L, "jass.globals", globals::open);
		register_preload_lib(L, "jass.japi",    japi::open);
		register_preload_lib(L, "jass.hook",    hook::open);
		register_preload_lib(L, "jass.runtime", runtime::open);
		register_preload_lib(L, "jass.debug",   debug::open);
		register_preload_lib(L, "jass.message", message::open);
		register_preload_lib(L, "jass.sleep",   sleep::open);
		register_preload_lib(L, "jass.storm",   storm::open);
		register_preload_lib(L, "jass.slk",     slk::open);

		// Initialize type metatables
		jreal::init(L);
		jhandle_ud_make_mt(L);
		jhandle_lud_make_mt(L);
		jarray_make_mt(L);

		return 0;
	}
}
