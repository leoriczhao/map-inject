// bee/registry.h — compatibility shim
// Minimal registry access for YDWE compatibility.
#pragma once

#include <string>
#include <windows.h>

namespace bee {
    class registry {
    public:
        static std::string read_string(HKEY root, const char* subkey, const char* value_name) {
            HKEY hkey;
            if (RegOpenKeyExA(root, subkey, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
                return "";
            char buf[1024] = {0};
            DWORD size = sizeof(buf);
            DWORD type = REG_SZ;
            if (RegQueryValueExA(hkey, value_name, nullptr, &type, (LPBYTE)buf, &size) != ERROR_SUCCESS) {
                RegCloseKey(hkey);
                return "";
            }
            RegCloseKey(hkey);
            return std::string(buf);
        }
    };
}
