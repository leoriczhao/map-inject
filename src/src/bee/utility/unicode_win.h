// bee/utility/unicode_win.h — compatibility shim
// YDWE uses bee::widen / bee::u2w from an older bee.lua version.
// Current bee.lua moved these to bee/win/unicode.h.
#pragma once

#include <string>
#include <string_view>
#include <windows.h>

namespace bee {
    inline std::wstring widen(const char* s, size_t len, unsigned codepage = CP_UTF8) {
        if (!s || len == 0) return L"";
        int wlen = MultiByteToWideChar(codepage, 0, s, (int)len, nullptr, 0);
        if (wlen <= 0) return L"";
        std::wstring ws(wlen, L'\0');
        MultiByteToWideChar(codepage, 0, s, (int)len, &ws[0], wlen);
        return ws;
    }
    inline std::wstring widen(const std::string& s, unsigned codepage = CP_UTF8) {
        return widen(s.data(), s.size(), codepage);
    }
    inline std::string narrow(const wchar_t* s, size_t len, unsigned codepage = CP_UTF8) {
        if (!s || len == 0) return "";
        int mlen = WideCharToMultiByte(codepage, 0, s, (int)len, nullptr, 0, nullptr, nullptr);
        if (mlen <= 0) return "";
        std::string ms(mlen, '\0');
        WideCharToMultiByte(codepage, 0, s, (int)len, &ms[0], mlen, nullptr, nullptr);
        return ms;
    }
    inline std::string narrow(const std::wstring& s, unsigned codepage = CP_UTF8) {
        return narrow(s.data(), s.size(), codepage);
    }
    // u2w: UTF-8 to wchar_t (used by open_lua_engine.cpp)
    inline std::wstring u2w(const char* s) { return widen(s, strlen(s), CP_UTF8); }
    inline std::wstring u2w(const std::string& s) { return widen(s, CP_UTF8); }
    inline std::wstring u2w(std::string_view s) { return widen(s.data(), s.size(), CP_UTF8); }
    // w2u: wchar_t to UTF-8
    inline std::string w2u(const wchar_t* s) { return narrow(s, wcslen(s), CP_UTF8); }
    inline std::string w2u(const std::wstring& s) { return narrow(s, CP_UTF8); }
}
