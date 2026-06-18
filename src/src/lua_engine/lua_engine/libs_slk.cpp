// libs_slk.cpp — jass.slk module (SlkLib version)
//
// Provides lazy-loaded access to game data tables (ability, unit, item, etc.)
// by reading SLK/TXT files from the game's MPQ archives via SlkLib + Storm API.
//
// Usage:
//   local slk = require "jass.slk"
//   print(slk.ability["AHbz"].code)  --> "AHbz"
//   print(slk.unit["hfoo"].race)     --> "human"

#include <lua.hpp>
#include <slk/ObjectManager.hpp>
#include <slk/storm_adapter.hpp>
#include <slk/table/SlkTable.hpp>
#include <string>
#include <memory>
#include <cstdio>

namespace warcraft3::lua_engine::slk {

// ── Singleton ObjectManager ────────────────────────────────────

static ::slk::StormAdapter& get_storm_adapter()
{
	static ::slk::StormAdapter adapter;
	return adapter;
}

static ::slk::ObjectManager& get_object_manager()
{
	static ::slk::ObjectManager mgr(get_storm_adapter());
	return mgr;
}

// ── Type mapping ───────────────────────────────────────────────

enum slk_type_index {
	SLK_ABILITY = 0,
	SLK_BUFF,
	SLK_UNIT,
	SLK_ITEM,
	SLK_UPGRADE,
	SLK_DOODAD,
	SLK_DESTRUCTABLE,
	SLK_TYPE_COUNT
};

static const char* slk_type_names[SLK_TYPE_COUNT] = {
	"ability", "buff", "unit", "item", "upgrade", "doodad", "destructable"
};

// ── Cached SlkTable per type ───────────────────────────────────

static const char* SLK_CACHE_KEY = "_SLK_OBJECT_CACHE";

static ::slk::SlkTable* get_or_load_table(lua_State* L, int type_idx)
{
	// Check registry cache
	lua_getfield(L, LUA_REGISTRYINDEX, SLK_CACHE_KEY);
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, SLK_CACHE_KEY);
	}
	lua_rawgeti(L, -1, type_idx + 1);
	if (!lua_isnil(L, -1)) {
		::slk::SlkTable* ptr = (::slk::SlkTable*)lua_touserdata(L, -1);
		lua_pop(L, 2);  // pop userdata and cache table
		return ptr;
	}
	lua_pop(L, 2);  // pop nil and cache table

	// Load from ObjectManager
	auto& mgr = get_object_manager();
	std::unique_ptr<::slk::SlkTable> table(new ::slk::SlkTable);

	switch (type_idx) {
	case SLK_ABILITY:
		mgr.load_ability(*table);
		break;
	case SLK_BUFF:
		mgr.load_buff(*table);
		break;
	case SLK_UNIT:
		mgr.load_unit(*table);
		break;
	case SLK_ITEM:
		mgr.load_item(*table);
		break;
	case SLK_UPGRADE:
		mgr.load_upgrde(*table);
		break;
	case SLK_DOODAD:
		mgr.load_doodad(*table);
		break;
	case SLK_DESTRUCTABLE:
		mgr.load_destructable(*table);
		break;
	default:
		return nullptr;
	}

	::slk::SlkTable* raw = table.release();

	// Store in registry cache as light userdata
	lua_getfield(L, LUA_REGISTRYINDEX, SLK_CACHE_KEY);
	lua_pushlightuserdata(L, raw);
	lua_rawseti(L, -2, type_idx + 1);
	lua_pop(L, 1);

	return raw;
}

// ── Lua metamethods ────────────────────────────────────────────

// row.__index(self, field_name) -> value
// self is a light userdata pointing to SlkSingle
static int row_index(lua_State* L)
{
	::slk::SlkSingle* single = (::slk::SlkSingle*)lua_touserdata(L, 1);
	if (!single) { lua_pushnil(L); return 1; }

	const char* field = luaL_checkstring(L, 2);
	auto it = single->find(field);
	if (it == single->end()) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushstring(L, it->second.to_string().c_str());
	return 1;
}

// type.__index(self, object_id) -> row proxy (light userdata + metatable)
static int type_index(lua_State* L)
{
	int type_idx = -1;

	// Get type_idx from metatable
	lua_getmetatable(L, 1);
	lua_getfield(L, -1, "_type_idx");
	type_idx = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	if (type_idx < 0 || type_idx >= SLK_TYPE_COUNT) {
		lua_pushnil(L);
		return 1;
	}

	const char* obj_id = luaL_checkstring(L, 2);

	::slk::SlkTable* table = get_or_load_table(L, type_idx);
	if (!table) {
		lua_pushnil(L);
		return 1;
	}

	// Search by string key (iterate for compatibility with SlkTable hash)
	auto it = table->end();
	for (auto i = table->begin(); i != table->end(); ++i) {
		if (i->first.to_string() == obj_id) {
			it = i;
			break;
		}
	}
	if (it == table->end()) {
		lua_pushnil(L);
		return 1;
	}

	// Create a userdata proxy that holds a pointer to the SlkSingle
	// and has a metatable with __index = row_index
	::slk::SlkSingle* single_ptr = &it->second;

	// Use a full userdata with metatable so we can attach __index
	lua_newuserdata(L, sizeof(void*));  // placeholder
	*(void**)lua_touserdata(L, -1) = single_ptr;

	// Create or reuse the row metatable
	lua_newtable(L);
	lua_pushcfunction(L, row_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	return 1;
}

// ── Module entry ───────────────────────────────────────────────

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
