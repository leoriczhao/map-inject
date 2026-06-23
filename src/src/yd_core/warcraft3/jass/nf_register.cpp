// nf_register.cpp — Callback Mode Adaptation
//
// Original YDWE: hooks StormAlloc + TlsGetValue to detect JASS VM initialization.
// In callback mode, the VM is already initialized when our DLLs load,
// so we skip the hooks and directly trigger nfunction_add/nfunction_hook.

#include <warcraft3/jass/nf_register.h>
#include <warcraft3/war3_searcher.h>

namespace warcraft3::jass {
    void nfunction_add();
    void nfunction_hook();
}

namespace warcraft3::jass::nf_register {
    base::signal<void, void> event_hook;
    base::signal<void, void> event_add;

    static bool s_initialized = false;

    bool initialize()
    {
        if (s_initialized)
            return false;
        s_initialized = true;

        // In callback mode, the JASS VM is already running.
        // Directly trigger the add and hook events.
        event_add();
        nfunction_add();

        event_hook();
        nfunction_hook();

        return true;
    }

    // Allow external code to re-trigger registration
    // (e.g., after a new DLL loads and registers more japi_add/japi_hook entries)
    void flush()
    {
        nfunction_add();
        nfunction_hook();
    }

    // flush_add_only: register new natives without hooking existing ones.
    // Safe to call during callback context (nfunction_hook modifies hash table
    // which is not available during callback).
    void flush_add_only()
    {
        nfunction_add();
    }
}
