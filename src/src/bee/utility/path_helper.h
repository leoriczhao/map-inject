// bee/utility/path_helper.h — compatibility shim
#pragma once

#include <string>
#include <windows.h>

namespace bee {
    namespace path_helper {
        inline std::string dll_path() {
            char buf[MAX_PATH] = {0};
            GetModuleFileNameA(nullptr, buf, MAX_PATH);
            return std::string(buf);
        }
    }
}
