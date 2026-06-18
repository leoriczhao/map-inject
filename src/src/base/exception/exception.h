#pragma once

#include <stdexcept>
#include <string>
#include <cstdarg>
#include <cstdio>

namespace base {

	class exception : public std::runtime_error
	{
	public:
		// Printf-style constructor
		template <typename... Args>
		exception(const char* fmt, Args... args)
			: std::runtime_error(format(fmt, args...))
		{ }

	private:
		static std::string format(const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			int size = _vscprintf(fmt, args);
			va_end(args);

			if (size <= 0) return std::string(fmt);

			std::string result(size, '\0');
			va_start(args, fmt);
			vsnprintf(result.data(), size + 1, fmt, args);
			va_end(args);

			return result;
		}
	};

} // namespace base
