// GameInfo.cpp — Game info natives (GetMapName, GetGameVersion, GetPluginVersion)
// Ported from builtin-japi war3map.j API surface.

#include <warcraft3/jass.h>
#include <warcraft3/jass/hook.h>
#include <warcraft3/war3_searcher.h>
#include <cstdio>

namespace warcraft3::japi {

	static char g_map_name_buf[512];

	// GetMapName() -> string
	// Returns the current map name. Reads from war3map.w3i or uses GetObjectName.
	static jass::jstring_t __cdecl GetMapName()
	{
		// Try reading from the JASS global "mapName" if it exists
		// For now, use a simple approach: call GetObjectName on the map
		const char* name = jass::from_string((jass::jstring_t)jass::call("GetObjectName"));
		if (name && name[0]) {
			return jass::create_string(name);
		}
		return jass::create_string("Unknown Map");
	}

	// GetGameVersion() -> integer
	// Returns the War3 version number (e.g. 6374 for 1.27a, 6401 for 1.27b)
	static int32_t __cdecl GetGameVersion()
	{
		return (int32_t)warcraft3::get_war3_searcher().get_version();
	}

	// GetPluginVersion() -> string
	// Returns the plugin version string.
	static jass::jstring_t __cdecl GetPluginVersion()
	{
		return jass::create_string("1.0.0");
	}

	void InitializeGameInfo()
	{
		jass::japi_add((uintptr_t)GetMapName, "GetMapName", "()S");
		jass::japi_add((uintptr_t)GetGameVersion, "GetGameVersion", "()I");
		jass::japi_add((uintptr_t)GetPluginVersion, "GetPluginVersion", "()S");
	}
}
