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

	// Load war3map.lua from MPQ and execute it.
	// Called lazily (not during Initialize) to avoid corrupting callback exploit state.
	static bool g_script_loaded = false;

	void load_script()
	{
		if (g_script_loaded) return;
		g_script_loaded = true;

		// Sync log
		{ HANDLE h = CreateFileA("C:\\ProgramData\\japi_sync.log", FILE_APPEND_DATA,
			FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		  if (h != INVALID_HANDLE_VALUE) { DWORD bw; const char* m = "load_script() START\r\n"; WriteFile(h, m, (DWORD)strlen(m), &bw, NULL); CloseHandle(h); } }

		lua_State* L = getMainL();
		if (!L) return;

		const char* buf = 0;
		size_t      len = 0;
		if (storm_s::instance().load_file("script\\war3map.lua", (const void**)&buf, &len))
		{
			if (luaL_loadbuffer(L, buf, len, "@script\\war3map.lua") != LUA_OK) {
				const char* err = lua_tostring(L, -1);
				printf("[japi-lua] load error: %s\n", err);
				{ HANDLE h = CreateFileA("C:\\ProgramData\\japi_sync.log", FILE_APPEND_DATA,
					FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				  if (h != INVALID_HANDLE_VALUE) { DWORD bw; std::string m = std::string("load_script() LUA LOAD ERROR: ") + (err?err:"null") + "\r\n"; WriteFile(h, m.c_str(), (DWORD)m.size(), &bw, NULL); CloseHandle(h); } }
				lua_pop(L, 1);
				storm_s::instance().unload_file(buf);
			}
			else
			{
				safe_call(L, 0, 0, true);
				storm_s::instance().unload_file(buf);
				printf("[japi-lua] war3map.lua loaded\n");
				{ HANDLE h = CreateFileA("C:\\ProgramData\\japi_sync.log", FILE_APPEND_DATA,
					FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				  if (h != INVALID_HANDLE_VALUE) { DWORD bw; const char* m = "load_script() OK\r\n"; WriteFile(h, m, (DWORD)strlen(m), &bw, NULL); CloseHandle(h); } }
			}
		}
		else
		{
			printf("[japi-lua] war3map.lua not found in MPQ\n");
			{ HANDLE h = CreateFileA("C:\\ProgramData\\japi_sync.log", FILE_APPEND_DATA,
				FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			  if (h != INVALID_HANDLE_VALUE) { DWORD bw; const char* m = "load_script() NOT FOUND in MPQ\r\n"; WriteFile(h, m, (DWORD)strlen(m), &bw, NULL); CloseHandle(h); } }
		}
	}

	// Bridge handler: LoadScript() — called from JASS to trigger lazy loading
	static uint32_t LoadScript_handler(const uint32_t*, size_t) {
		load_script();
		return 0;
	}

	// Callback mode: create Lua state only (no JASS calls).
	// war3map.lua loading is deferred to LoadScript bridge handler.
	void initialize()
	{
		lua_State* L = getMainL();
		if (!L) {
			DBG_LOG("getMainL() returned NULL");
			return;
		}
		DBG_LOG("getMainL() OK, L=%p", L);

		DBG_LOG("Lua VM created, script loading deferred to LoadScript");
	}

	// Bridge handler for LoadScript — called from JASS via UnitId("LoadScript")
	uint32_t LoadScript_bridge(const uint32_t*, size_t) {
		load_script();
		return 0;
	}
}
