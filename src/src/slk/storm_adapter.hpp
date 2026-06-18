#pragma once

#include <slk/InterfaceStorm.hpp>
#include <lua_engine/lua_engine/storm.h>

namespace slk
{
	// Adapts warcraft3::storm_dll (Storm API) to the slk::InterfaceStorm interface
	// expected by SlkLib's ObjectManager.
	class StormAdapter : public InterfaceStorm
	{
	public:
		bool has(std::string const& path) override
		{
			return warcraft3::storm_s::instance().has_file(path.c_str());
		}

		std::string load(std::string const& path, error_code& ec) override
		{
			const void* buf = nullptr;
			size_t len = 0;
			if (!warcraft3::storm_s::instance().load_file(path.c_str(), &buf, &len))
			{
				ec = 1;
				return std::string();
			}

			std::string result((const char*)buf, len);
			warcraft3::storm_s::instance().unload_file(buf);
			ec = 0;
			return result;
		}
	};
}
