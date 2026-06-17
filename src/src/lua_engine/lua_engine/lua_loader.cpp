// lua_loader.cpp — Callback Mode Adaptation
//
// Original YDWE: hooks war3map.j loading via virtual_mpq::force_watch,
// then loads war3map.lua when a new map is detected.
// In callback mode, the map is already loaded, so we directly create
// the Lua state and load war3map.lua from the MPQ.

#include "callback.h"
#include "fix_baselib.h"
#include "lua_loader.h"
#include "storm.h"
#include "open_lua_engine.h"
#include "libs_runtime.h"
#include <lua.hpp>
#include <warcraft3/jass.h>
#include <warcraft3/jass/func_value.h>
#include <warcraft3/jass/hook.h>
#include <base/util/singleton.h>
#include <base/hook/fp_call.h>
#include <string_view>
#include <base/util/string_algorithm.h>
#include <cstdio>
#include "debug_log.h"

namespace warcraft3::lua_engine::lua_loader {

	static lua_State* mainL = 0;

	lua_State* getMainL()
	{
		if (!mainL) {
			lua_State* L = newstate();
			if (L) {
				luaL_openlibs(L);
				open_lua_engine(L);
				runtime::initialize();
			}
			mainL = L;
		}
		return mainL;
	}

	uintptr_t RealCheat = 0;
	void __cdecl FakeCheat(jass::jstring_t cheat_str)
	{
		const char* cheat = jass::from_string(cheat_str);

		if (!cheat)
		{
			base::c_call<uint32_t>(RealCheat, cheat_str);
			return;
		}

		std::string_view cheat_s = cheat;

		if (cheat_s.substr(0, 9) == "exec-lua:")
		{
			cheat_s = cheat_s.substr(9);
			base::algorithm::trim(cheat_s);
			if (cheat_s.size() >= 2 && cheat_s[0] == '"' && cheat_s[cheat_s.size() - 1] == '"')
			{
				cheat_s = cheat_s.substr(1, cheat_s.size() - 2);
			}
			lua_State* L = getMainL();
			lua_getglobal(L, "require");
			lua_pushlstring(L, cheat_s.data(), cheat_s.size());
			safe_call(L, 1, 1, true);
		}

		base::c_call<uint32_t>(RealCheat, cheat_str);
	}

	jass::jstring_t __cdecl EXExecuteScript(jass::jstring_t script)
	{
		lua_State* L = getMainL();

		const char* raw = jass::from_trigstring(jass::from_string(script));
		char buf[4096];
		snprintf(buf, sizeof(buf), "return (%s)", raw ? raw : "");
		size_t len = strlen(buf);

		if (luaL_loadbuffer(L, buf, len, buf) != LUA_OK)
		{
			printf("%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			return 0;
		}

		if (LUA_OK != safe_call(L, 0, 1, true))
		{
			return 0;
		}

		jass::jstring_t result = 0;
		if (lua_isstring(L, -1))
		{
			result = jass::create_string(lua_tostring(L, -1));
		}
		lua_pop(L, 1);
		return result;
	}

	// Callback mode: directly create Lua state and load war3map.lua
	void initialize()
	{
		lua_State* L = getMainL();
		if (!L) {
			DBG_LOG("getMainL() returned NULL");
			return;
		}

		DBG_LOG("getMainL() OK, L=%p", L);

		// Try to load war3map.lua from the MPQ
		const char* buf = 0;
		size_t      len = 0;
		if (storm_s::instance().load_file("script\\war3map.lua", (const void**)&buf, &len))
		{
			DBG_LOG("storm load OK, len=%zu", len);

			if (luaL_loadbuffer(L, buf, len, "@script\\war3map.lua") != LUA_OK) {
				const char* err = lua_tostring(L, -1);
				DBG_LOG("LUA LOAD ERROR: %s", err ? err : "null");
				lua_pop(L, 1);
				storm_s::instance().unload_file(buf);
			}
			else
			{
				DBG_LOG("luaL_loadbuffer OK, calling safe_call...");
				safe_call(L, 0, 0, true);
				storm_s::instance().unload_file(buf);
				DBG_LOG("war3map.lua loaded OK");
			}
		}
		else
		{
			DBG_LOG("storm load FAILED - war3map.lua not in MPQ");
		}

		// Register EXExecuteScript (always, even if war3map.lua is missing)
		jass::table_hook("Cheat", (uintptr_t*)&RealCheat, (uintptr_t)FakeCheat);
		jass::japi_table_add((uintptr_t)EXExecuteScript, "EXExecuteScript", "(S)S");
	}
}
