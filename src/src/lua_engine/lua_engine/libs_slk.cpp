// libs_slk.cpp — jass.slk module
//
// Provides lazy-loaded access to game data tables (ability, unit, item, etc.)
// by reading SLK files from the game's MPQ archives via Storm API.
//
// Usage:
//   local slk = require "jass.slk"
//   print(slk.ability["AHbz"].code)  --> "AHbz"
//   print(slk.unit["hfoo"].race)     --> "human"

#include <lua.hpp>
#include "storm.h"
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace warcraft3::lua_engine::slk {

// ── SLK parser ───────────────────────────────────────────────

struct slk_table {
	std::vector<std::string> headers;
	std::vector<std::vector<std::string>> rows;
	std::map<std::string, int> col_index;   // header name -> 0-based column
};

static std::string unquote(const std::string& s)
{
	if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
		return s.substr(1, s.size() - 2);
	return s;
}

static bool parse_slk(const char* buf, size_t len, slk_table& out)
{
	std::string text(buf, len);
	int max_x = 0, max_y = 0;

	// First pass: find dimensions from B record
	size_t pos = 0;
	while (pos < text.size()) {
		size_t eol = text.find('\n', pos);
		if (eol == std::string::npos) eol = text.size();
		std::string line = text.substr(pos, eol - pos);
		pos = eol + 1;
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty() || line[0] != 'B') continue;

		// B;X<cols>;Y<rows>
		size_t p = 2;
		while (p < line.size()) {
			if (line[p] == 'X') max_x = atoi(line.c_str() + p + 1);
			else if (line[p] == 'Y') max_y = atoi(line.c_str() + p + 1);
			size_t semi = line.find(';', p);
			if (semi == std::string::npos) break;
			p = semi + 1;
		}
		break;
	}

	if (max_x <= 0 || max_y <= 0) return false;

	out.headers.resize(max_x);
	out.rows.resize(max_y - 1);
	for (auto& row : out.rows) row.resize(max_x);

	// Second pass: parse C records
	pos = 0;
	int cur_x = 0, cur_y = 0;
	while (pos < text.size()) {
		size_t eol = text.find('\n', pos);
		if (eol == std::string::npos) eol = text.size();
		std::string line = text.substr(pos, eol - pos);
		pos = eol + 1;
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty()) continue;
		if (line[0] == 'E') break;
		if (line[0] != 'C') continue;

		// Parse C record fields
		int cx = cur_x, cy = cur_y;
		std::string value;
		bool has_value = false;

		size_t p = 2;
		while (p < line.size()) {
			char field = line[p];
			size_t val_start = p + 1;
			size_t semi = line.find(';', val_start);
			if (semi == std::string::npos) semi = line.size();
			std::string val = line.substr(val_start, semi - val_start);

			if (field == 'X') cx = atoi(val.c_str());
			else if (field == 'Y') cy = atoi(val.c_str());
			else if (field == 'K' || field == 'E') { value = unquote(val); has_value = true; }

			p = semi + 1;
		}

		if (!has_value) continue;

		if (cy == 1 && cx >= 1 && cx <= max_x) {
			out.headers[cx - 1] = value;
			out.col_index[value] = cx - 1;
		} else if (cy >= 2 && cy <= max_y) {
			int ri = cy - 2;
			if (cx >= 1 && cx <= max_x && ri < (int)out.rows.size())
				out.rows[ri][cx - 1] = value;
		}
		cur_x = cx;
	}

	return true;
}

// ── File paths for each type ─────────────────────────────────

static const char* slk_files[] = {
	"Units\\AbilityData.slk",
	"Units\\AbilityBuffData.slk",
	"Units\\UnitData.slk",
	"Units\\ItemData.slk",
	"Units\\UpgradeData.slk",
	"Doodads\\Doodads.slk",
	"Units\\DestructableData.slk",
};

static const char* slk_type_names[] = {
	"ability", "buff", "unit", "item", "upgrade", "doodad", "destructable"
};

static const int SLK_TYPE_COUNT = 7;

// ── Lua metamethods ──────────────────────────────────────────

// Registry keys for per-type cached data
// Stored as: _SLK_DATA[type_idx] = { col_index_ref, data_ref, id_map_ref }

static const char* SLK_CACHE_KEY = "_SLK_CACHE";

// Ensure the cache table exists in registry, return it on stack
static void get_cache(lua_State* L)
{
	lua_getfield(L, LUA_REGISTRYINDEX, SLK_CACHE_KEY);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, SLK_CACHE_KEY);
	}
}

// Load and parse SLK for a given type index. Returns true if data is available.
// Leaves col_index_ref, data_ref, id_map_ref on stack (3 values).
static bool ensure_loaded(lua_State* L, int type_idx)
{
	get_cache(L);
	lua_rawgeti(L, -1, type_idx + 1);
	if (lua_istable(L, -1)) {
		// Already loaded — get refs from cache
		lua_getfield(L, -1, "col_index_ref");
		lua_getfield(L, -2, "data_ref");
		lua_getfield(L, -3, "id_map_ref");
		lua_remove(L, -4);  // remove cache entry
		lua_remove(L, -4);  // remove cache table
		return true;
	}
	lua_pop(L, 2);  // pop nil and cache table

	// Load from Storm
	const void* buf = nullptr;
	size_t len = 0;
	if (!storm_s::instance().load_file(slk_files[type_idx], &buf, &len)) {
		return false;
	}

	slk_table table;
	bool ok = parse_slk((const char*)buf, len, table);
	storm_s::instance().unload_file(buf);
	if (!ok) return false;

	// Create col_index Lua table (header_name -> 0-based column index)
	lua_newtable(L);
	for (auto& kv : table.col_index) {
		lua_pushstring(L, kv.first.c_str());
		lua_pushinteger(L, kv.second);
		lua_rawset(L, -3);
	}
	int col_index_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// Create data Lua table (array of row arrays)
	lua_newtable(L);
	for (int r = 0; r < (int)table.rows.size(); r++) {
		lua_newtable(L);
		for (int c = 0; c < (int)table.rows[r].size(); c++) {
			lua_pushstring(L, table.rows[r][c].c_str());
			lua_rawseti(L, -2, c + 1);
		}
		lua_rawseti(L, -2, r + 1);
	}
	int data_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// Create id_map Lua table (object_id -> 1-based row index)
	lua_newtable(L);
	for (int r = 0; r < (int)table.rows.size(); r++) {
		if (!table.rows[r].empty() && !table.rows[r][0].empty()) {
			lua_pushstring(L, table.rows[r][0].c_str());
			lua_pushinteger(L, r + 1);
			lua_rawset(L, -3);
		}
	}
	int id_map_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	// Store in cache
	get_cache(L);
	lua_newtable(L);
	lua_pushinteger(L, col_index_ref); lua_setfield(L, -2, "col_index_ref");
	lua_pushinteger(L, data_ref);      lua_setfield(L, -2, "data_ref");
	lua_pushinteger(L, id_map_ref);    lua_setfield(L, -2, "id_map_ref");
	lua_rawseti(L, -2, type_idx + 1);
	lua_pop(L, 1);  // pop cache

	// Push refs for caller
	lua_rawgeti(L, LUA_REGISTRYINDEX, col_index_ref);
	lua_rawgeti(L, LUA_REGISTRYINDEX, data_ref);
	lua_rawgeti(L, LUA_REGISTRYINDEX, id_map_ref);
	return true;
}

// row.__index(self, field_name) -> value
static int row_index(lua_State* L)
{
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "_col_index");
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushnil(L);
		return 1;
	}
	int col = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);  // pop result and _col_index

	lua_getfield(L, -1, "_data");
	lua_rawgeti(L, -1, col + 1);  // 1-indexed
	return 1;
}

// type.__index(self, object_id) -> row proxy
static int type_index(lua_State* L)
{
	const char* obj_id = luaL_checkstring(L, 2);
	int type_idx = -1;

	// Find type index from metatable
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "_type_idx");
	type_idx = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	if (type_idx < 0 || type_idx >= SLK_TYPE_COUNT) {
		lua_pushnil(L);
		return 1;
	}

	// Ensure SLK data is loaded
	if (!ensure_loaded(L, type_idx)) {
		lua_pushnil(L);
		return 1;
	}
	// Stack: col_index_ref, data_ref, id_map_ref
	int id_map_ref_idx = lua_gettop(L);
	int data_ref_idx = id_map_ref_idx - 1;
	int col_index_ref_idx = id_map_ref_idx - 2;

	// Look up object ID in id_map
	lua_pushvalue(L, 2);  // obj_id
	lua_rawget(L, id_map_ref_idx);
	if (lua_isnil(L, -1)) {
		lua_pushnil(L);
		return 1;
	}
	int row_idx = (int)lua_tointeger(L, -1);
	lua_pop(L, 1);

	// Create row proxy table
	lua_newtable(L);

	// Metatable: { __index = row_index, _col_index = ..., _data = row_data }
	lua_newtable(L);
	lua_pushcfunction(L, row_index);
	lua_setfield(L, -2, "__index");

	// Store col_index table reference
	lua_pushvalue(L, col_index_ref_idx);
	lua_setfield(L, -2, "_col_index");

	// Store the specific row data
	lua_rawgeti(L, data_ref_idx, row_idx);
	lua_setfield(L, -2, "_data");

	lua_setmetatable(L, -2);

	return 1;
}

// ── Module entry ─────────────────────────────────────────────

int open(lua_State* L)
{
	lua_newtable(L);  // slk table

	for (int i = 0; i < SLK_TYPE_COUNT; i++) {
		lua_newtable(L);  // type proxy

		lua_newtable(L);  // metatable
		lua_pushinteger(L, i);
		lua_setfield(L, -2, "_type_idx");
		lua_pushcfunction(L, type_index);
		lua_setfield(L, -2, "__index");
		lua_setmetatable(L, -2);

		lua_setfield(L, -2, slk_type_names[i]);
	}

	return 1;
}

}  // namespace warcraft3::lua_engine::slk
