// CodeRunLogs.cpp — open_code_run_logs native stub
//
// The builtin-japi's open_code_run_logs feature:
// 1. JASS side builds a function dependency table via Lua (parsing war3map.j)
// 2. JASS calls UnitId("open_code_run_logs") with boolean parameter
// 3. C++ side hooks JASS function calls and logs tracked functions
//
// This is a stub implementation. The full dependency-graph parsing
// and JASS call interception will be added later.

#include <warcraft3/jass.h>
#include <warcraft3/jass/hook.h>
#include <cstdio>
#include "../../debug_log.h"

namespace warcraft3::japi {

	static bool g_code_run_logs_enabled = false;

	// open_code_run_logs(open: boolean) -> void
	// Registered via bridge dispatch (UnitId hashtable RPC)
	uint32_t open_code_run_logs_handler(const uint32_t* args, size_t nargs)
	{
		bool open = (nargs >= 1) && (args[0] != 0);
		g_code_run_logs_enabled = open;
		DBG_LOG("[CodeRunLogs] %s", open ? "enabled" : "disabled");

		// Display status in-game
		const char* msg = open
			? "[japi] Code run logs: ON"
			: "[japi] Code run logs: OFF";
		jass::string_fake sf(msg);
		jass::call("DisplayTimedTextToPlayer",
			jass::call("GetLocalPlayer"), 0, 0, 5.0f, (jass::jstring_t)sf);

		return 0;
	}

	bool is_code_run_logs_enabled()
	{
		return g_code_run_logs_enabled;
	}

	void InitializeCodeRunLogs()
	{
		// Registered via bridge dispatch in dllmain.cpp
		// The handler is registered there since it needs the bridge module
	}
}
