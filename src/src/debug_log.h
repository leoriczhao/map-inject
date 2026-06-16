// debug_log.h — Lightweight debug logging for japi.dll
//
// Features:
//   - AllocConsole for real-time console output
//   - Async file writing via background thread + lock-free ring buffer
//   - Compile-time toggle: define JAPI_DEBUG to enable, otherwise all macros are no-ops
//
// Usage:
//   DBG_INIT();              // call once in DllMain or Initialize
//   DBG_LOG("msg %d", val);  // printf-style logging
//
// Build:
//   CMake: target_compile_definitions(yd_japi PRIVATE JAPI_DEBUG)

#pragma once

#ifdef JAPI_DEBUG

#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>

namespace japi_log {

// ── Console ─────────────────────────────────────────────────
inline void init_console() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    SetConsoleTitleA("japi debug");
}

// ── Async file writer ───────────────────────────────────────
// Single producer (DBG_LOG calls), single consumer (background thread).
// Ring buffer of fixed-size entries, flushed periodically.

struct LogEntry {
    char buf[512];
    int  len;
};

static const int RING_SIZE = 256;

struct LogRing {
    LogEntry entries[RING_SIZE];
    volatile LONG write_idx = 0;
    volatile LONG read_idx  = 0;

    // File + sync
    HANDLE  file       = INVALID_HANDLE_VALUE;
    HANDLE  thread     = NULL;
    HANDLE  stop_event = NULL;
    CRITICAL_SECTION cs;

    bool push(const char* data, int len) {
        LONG next = (write_idx + 1) % RING_SIZE;
        if (next == read_idx) return false;  // full, drop
        LogEntry& e = entries[write_idx];
        int n = len < 511 ? len : 511;
        memcpy(e.buf, data, n);
        e.buf[n] = '\0';
        e.len = n;
        InterlockedExchange(&write_idx, next);
        return true;
    }

    bool pop(LogEntry& out) {
        if (read_idx == write_idx) return false;
        out = entries[read_idx];
        InterlockedExchange(&read_idx, (read_idx + 1) % RING_SIZE);
        return true;
    }
};

static LogRing g_ring;

static DWORD WINAPI flush_thread(LPVOID) {
    while (WaitForSingleObject(g_ring.stop_event, 50) == WAIT_TIMEOUT) {
        LogEntry e;
        while (g_ring.pop(e)) {
            if (g_ring.file != INVALID_HANDLE_VALUE) {
                DWORD written;
                WriteFile(g_ring.file, e.buf, e.len, &written, NULL);
                WriteFile(g_ring.file, "\r\n", 2, &written, NULL);
            }
        }
    }
    // Final flush
    LogEntry e;
    while (g_ring.pop(e)) {
        if (g_ring.file != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(g_ring.file, e.buf, e.len, &written, NULL);
            WriteFile(g_ring.file, "\r\n", 2, &written, NULL);
        }
    }
    return 0;
}

inline void init_file() {
    g_ring.file = CreateFileA(
        "C:\\ProgramData\\japi_debug.txt",
        GENERIC_WRITE, FILE_SHARE_READ, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    g_ring.stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    g_ring.thread = CreateThread(NULL, 0, flush_thread, NULL, 0, NULL);
}

inline void shutdown() {
    if (g_ring.thread) {
        SetEvent(g_ring.stop_event);
        WaitForSingleObject(g_ring.thread, 2000);
        CloseHandle(g_ring.thread);
        g_ring.thread = NULL;
    }
    if (g_ring.stop_event) {
        CloseHandle(g_ring.stop_event);
        g_ring.stop_event = NULL;
    }
    if (g_ring.file != INVALID_HANDLE_VALUE) {
        CloseHandle(g_ring.file);
        g_ring.file = INVALID_HANDLE_VALUE;
    }
    FreeConsole();
}

inline void log(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    if (len <= 0) return;
    if (len > 511) len = 511;

    // Console (immediate)
    printf("%s\n", buf);

    // Ring buffer (async file)
    g_ring.push(buf, len);
}

}  // namespace japi_log

#define DBG_INIT()   do { japi_log::init_console(); japi_log::init_file(); } while(0)
#define DBG_LOG(...)  japi_log::log(__VA_ARGS__)
#define DBG_SHUTDOWN() japi_log::shutdown()

#else  // JAPI_DEBUG not defined

#define DBG_INIT()      ((void)0)
#define DBG_LOG(...)    ((void)0)
#define DBG_SHUTDOWN()  ((void)0)

#endif
