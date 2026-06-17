// libs_sleep.cpp — jass.sleep module
//
// Registers a global sleep(seconds) function that yields the Lua coroutine
// via TriggerSleepAction. Requires runtime.sleep = true to function.

#include <lua.hpp>
#include <warcraft3/jass.h>
#include <warcraft3/jass/func_value.h>
#include <cstdio>

namespace warcraft3::lua_engine::sleep {

	// sleep(seconds) — yields the current Lua coroutine for the given duration.
	// Calls TriggerSleepAction which is marked as sleep-capable in libs_common.cpp.
	// When runtime.sleep == true, jass_call_closure detects has_sleep on the
	// JASS VM thread and yields the Lua coroutine automatically.
	// When runtime.sleep == false, calling this prints a warning and returns nil.
	static int sleep_func(lua_State* L)
	{
		float seconds = (float)luaL_checknumber(L, 1);
		if (seconds <= 0) seconds = 0.001f;

		jass::call("TriggerSleepAction", seconds);
		return 0;
	}

	int open(lua_State* L)
	{
		lua_newtable(L);

		lua_pushstring(L, "sleep");
		lua_pushcclosure(L, sleep_func, 0);
		lua_rawset(L, -3);

		// Also register global "sleep" function
		lua_pushglobaltable(L);
		lua_pushstring(L, "sleep");
		lua_pushcclosure(L, sleep_func, 0);
		lua_rawset(L, -3);
		lua_pop(L, 1);

		return 1;
	}
}
