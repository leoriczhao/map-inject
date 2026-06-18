// stubs.cpp — Provides missing symbols for the minimal build

#include <windows.h>
#include <cstdint>
#include <cstring>
#include <base/win/pe_reader.h>
#include <warcraft3/jass.h>

// === base::win::pe_reader ===
namespace base::win {
    pe_reader::pe_reader() : module_(0) {}
    pe_reader::pe_reader(HMODULE mod) : module_((uintptr_t)mod) {}
    void pe_reader::set_module(HMODULE mod) { module_ = (uintptr_t)mod; }
    HMODULE pe_reader::module() const { return (HMODULE)module_; }
    size_t pe_reader::module_size() const {
        if (!module_) return 0;
        auto dos = get_dos_header();
        auto nt = get_nt_headers();
        if (!dos || !nt) return 0;
        return nt->OptionalHeader.SizeOfImage;
    }
    uintptr_t pe_reader::rva_to_addr(uintptr_t rva) const {
        return module_ + rva;
    }
    PIMAGE_DOS_HEADER pe_reader::get_dos_header() const {
        return (PIMAGE_DOS_HEADER)module_;
    }
    PIMAGE_NT_HEADERS pe_reader::get_nt_headers() const {
        auto dos = get_dos_header();
        if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
        return (PIMAGE_NT_HEADERS)(module_ + dos->e_lfanew);
    }
    uintptr_t pe_reader::get_directory_entry(uint32_t directory) const {
        auto nt = get_nt_headers();
        if (!nt || directory >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES) return 0;
        return nt->OptionalHeader.DataDirectory[directory].VirtualAddress;
    }
    uint32_t pe_reader::get_directory_entry_size(uint32_t directory) const {
        auto nt = get_nt_headers();
        if (!nt || directory >= IMAGE_NUMBEROF_DIRECTORY_ENTRIES) return 0;
        return nt->OptionalHeader.DataDirectory[directory].Size;
    }
    uint32_t pe_reader::get_num_sections() const {
        auto nt = get_nt_headers();
        if (!nt) return 0;
        return nt->FileHeader.NumberOfSections;
    }
    PIMAGE_SECTION_HEADER pe_reader::get_section_by_name(const char* name) const {
        auto nt = get_nt_headers();
        if (!nt) return nullptr;
        auto section = IMAGE_FIRST_SECTION(nt);
        for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
            if (strncmp((const char*)section[i].Name, name, IMAGE_SIZEOF_SHORT_NAME) == 0)
                return &section[i];
        }
        return nullptr;
    }
}

// === base::hook::replace_pointer ===
namespace base::hook {
    uintptr_t replace_pointer(uintptr_t address, uintptr_t new_value) {
        DWORD old_protect;
        uintptr_t old_value = 0;
        if (VirtualProtect((void*)address, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old_protect)) {
            old_value = *(uintptr_t*)address;
            *(uintptr_t*)address = new_value;
            VirtualProtect((void*)address, sizeof(uintptr_t), old_protect, &old_protect);
        }
        return old_value;
    }
}

// === base::console ===
namespace base::console {
    void enable() {
        AllocConsole();
    }
    void disable() {
        FreeConsole();
    }
    bool disable_close_button() {
        return true;
    }
}

// === base::hook::iat (minimal stubs) ===
namespace base::hook {
    uintptr_t iat(HMODULE module_handle, const char* dll_name, const char* api_name, uintptr_t new_function) {
        (void)module_handle; (void)dll_name; (void)api_name; (void)new_function;
        return 0;
    }
    uintptr_t iat(const wchar_t* module_name, const char* dll_name, const char* api_name, uintptr_t new_function) {
        (void)module_name; (void)dll_name; (void)api_name; (void)new_function;
        return 0;
    }
}

// === warcraft3::jass::from_real / to_real ===
namespace warcraft3::jass {
    float from_real(jreal_t val) {
        float result;
        memcpy(&result, &val, sizeof(float));
        return result;
    }
    jreal_t to_real(float val) {
        jreal_t result;
        memcpy(&result, &val, sizeof(float));
        return result;
    }
}

// === base::hook::install ===
namespace base::hook {
    bool install(uintptr_t* target, uintptr_t new_func, void** old_func) {
        if (old_func) *old_func = (void*)*target;
        DWORD old_protect;
        if (VirtualProtect(target, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old_protect)) {
            *target = new_func;
            VirtualProtect(target, sizeof(uintptr_t), old_protect, &old_protect);
            return true;
        }
        return false;
    }
}

// warcraft3::japi::initialize is defined in extensions/custom_natives.cpp
