// fmt/format.h — minimal compatibility shim
// Replaces {fmt} library with snprintf-based implementation.
#pragma once

#include <string>
#include <cstdio>
#include <cstdarg>

namespace fmt {

template <size_t N = 4096>
inline std::string format(const char* fmt_str, ...) {
    char buf[N];
    va_list args;
    va_start(args, fmt_str);
    vsnprintf(buf, N, fmt_str, args);
    va_end(args);
    return std::string(buf);
}

inline std::string format(const char* fmt_str) {
    return std::string(fmt_str);
}

} // namespace fmt
