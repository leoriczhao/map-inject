// api.cpp — D3D9 API handler implementations
//
// Each handler reads args from the JASS hashtable (via the bridge dispatch pattern)
// and calls into d3d9_renderer to manage fonts, text, and textures.
//
// The bridge dispatch reads args from ht slots 1..N and writes result to slot 0.
// For strings, the bridge passes the JASS string handle as an integer.
// We use jass::from_string() to convert (linked from yd_japi.dll at runtime).

#include "api.h"
#include "renderer.h"
#include <cstring>
#include <cstdio>

// We need jass::from_string() to convert JASS string handles to C strings.
// Since this DLL is separate from yd_japi.dll, we dynamically resolve it.
// Alternative: the bridge dispatch passes raw C strings for 'S' params.
//
// For the initial implementation, we'll use a simpler approach:
// The bridge dispatch's cpp_handler_fn receives args as uint32_t array.
// For 'S' type params, the bridge loads the string from the hashtable
// and passes a pointer to a static buffer (or we re-read from ht).
//
// Actually, looking at bridge_dispatch.cpp more carefully:
// For C++ handlers, it reads 'S' args as integers (jstring_t = uint32_t).
// The handler must call jass::from_string() itself.
//
// We need to resolve jass::from_string at runtime. Let's use GetProcAddress
// on the yd_japi.dll module, or simpler: pass a function pointer.

// For now, we'll use a helper that the Initialize function provides.
namespace {
    typedef const char* (*from_string_fn)(uint32_t);
    from_string_fn g_from_string = nullptr;

    // Helper to read a string from a uint32_t arg (jstring_t)
    const char* arg_to_string(uint32_t val) {
        if (g_from_string) {
            return g_from_string(val);
        }
        return "";
    }

    // Helper to reinterpret uint32_t as float
    float arg_to_float(uint32_t val) {
        float f;
        memcpy(&f, &val, sizeof(float));
        return f;
    }

    // Helper to reinterpret float as uint32_t
    uint32_t float_to_arg(float f) {
        uint32_t u;
        memcpy(&u, &f, sizeof(uint32_t));
        return u;
    }
}

namespace d3d9_api {

// ── Font handlers ─────────────────────────────────────────────

static uint32_t h_CreateFont(const uint32_t* args, size_t nargs) {
    (void)args; (void)nargs;
    return d3d9_renderer::create_font();
}

static uint32_t h_DestroyFont(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) d3d9_renderer::destroy_font((d3d9_renderer::font_handle_t)args[0]);
    return 0;
}

static uint32_t h_GetFontWidth(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return (uint32_t)d3d9_renderer::get_font_width((d3d9_renderer::font_handle_t)args[0]);
    return 0;
}

static uint32_t h_SetFontWidth(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_font_width((d3d9_renderer::font_handle_t)args[0], (int)args[1]);
    return 0;
}

static uint32_t h_GetFontHeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return (uint32_t)d3d9_renderer::get_font_height((d3d9_renderer::font_handle_t)args[0]);
    return 0;
}

static uint32_t h_SetFontHeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_font_height((d3d9_renderer::font_handle_t)args[0], (int)args[1]);
    return 0;
}

static uint32_t h_GetFontWeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return (uint32_t)d3d9_renderer::get_font_weight((d3d9_renderer::font_handle_t)args[0]);
    return 0;
}

static uint32_t h_SetFontWeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_font_weight((d3d9_renderer::font_handle_t)args[0], (int)args[1]);
    return 0;
}

static uint32_t h_SetFontItalic(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_font_italic((d3d9_renderer::font_handle_t)args[0], args[1] != 0);
    return 0;
}

static uint32_t h_GetFontFaceName(const uint32_t* args, size_t nargs) {
    // Returns a string — we need to store it and return a pointer.
    // The bridge dispatch expects a const char* for 'S' return type.
    // We store in a static buffer and return the pointer.
    static char buf[256];
    if (nargs >= 1) {
        const char* name = d3d9_renderer::get_font_facename((d3d9_renderer::font_handle_t)args[0]);
        strncpy(buf, name ? name : "", 255);
        buf[255] = '\0';
    }
    return (uint32_t)(uintptr_t)buf;
}

static uint32_t h_SetFontFaceName(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) {
        const char* name = arg_to_string(args[1]);
        d3d9_renderer::set_font_facename((d3d9_renderer::font_handle_t)args[0], name);
    }
    return 0;
}

// ── Text handlers ─────────────────────────────────────────────

static uint32_t h_CreateText(const uint32_t* args, size_t nargs) {
    // Spec: (ISRRRI)I
    if (nargs >= 6) {
        d3d9_renderer::font_handle_t font = (d3d9_renderer::font_handle_t)args[0];
        const char* str = arg_to_string(args[1]);
        float x = arg_to_float(args[2]);
        float y = arg_to_float(args[3]);
        float time = arg_to_float(args[4]);
        uint32_t color = args[5];
        return d3d9_renderer::create_text(font, str, x, y, time, color);
    }
    return 0;
}

static uint32_t h_GetTextString(const uint32_t* args, size_t nargs) {
    static char buf[4096];
    if (nargs >= 1) {
        const char* s = d3d9_renderer::get_text_string((d3d9_renderer::text_handle_t)args[0]);
        strncpy(buf, s ? s : "", 4095);
        buf[4095] = '\0';
    }
    return (uint32_t)(uintptr_t)buf;
}

static uint32_t h_SetTextString(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) {
        const char* str = arg_to_string(args[1]);
        d3d9_renderer::set_text_string((d3d9_renderer::text_handle_t)args[0], str);
    }
    return 0;
}

static uint32_t h_GetTextX(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_text_x((d3d9_renderer::text_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextX(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_text_x((d3d9_renderer::text_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextY(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_text_y((d3d9_renderer::text_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextY(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_text_y((d3d9_renderer::text_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextTime(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_text_time((d3d9_renderer::text_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextTime(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_text_time((d3d9_renderer::text_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextColor(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return d3d9_renderer::get_text_color((d3d9_renderer::text_handle_t)args[0]);
    return 0;
}

static uint32_t h_SetTextColor(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_text_color((d3d9_renderer::text_handle_t)args[0], args[1]);
    return 0;
}

// ── Texture handlers ──────────────────────────────────────────

static uint32_t h_CreateTexture(const uint32_t* args, size_t nargs) {
    // Spec: (SRRRRII)I
    if (nargs >= 7) {
        const char* path = arg_to_string(args[0]);
        float x = arg_to_float(args[1]);
        float y = arg_to_float(args[2]);
        float w = arg_to_float(args[3]);
        float h_val = arg_to_float(args[4]);
        uint32_t color = args[5];
        int level = (int)args[6];
        return d3d9_renderer::create_texture(path, x, y, w, h_val, color, level);
    }
    return 0;
}

static uint32_t h_DestroyTexture(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) d3d9_renderer::destroy_texture((d3d9_renderer::texture_handle_t)args[0]);
    return 0;
}

static uint32_t h_GetTextureX(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_x((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextureX(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_x((d3d9_renderer::texture_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextureY(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_y((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextureY(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_y((d3d9_renderer::texture_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextureWidth(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_width((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextureWidth(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_width((d3d9_renderer::texture_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextureHeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_height((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextureHeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_height((d3d9_renderer::texture_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTextureColor(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return d3d9_renderer::get_texture_color((d3d9_renderer::texture_handle_t)args[0]);
    return 0;
}

static uint32_t h_SetTextureColor(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_color((d3d9_renderer::texture_handle_t)args[0], args[1]);
    return 0;
}

static uint32_t h_GetTextureLevel(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return (uint32_t)d3d9_renderer::get_texture_level((d3d9_renderer::texture_handle_t)args[0]);
    return 0;
}

static uint32_t h_SetTextureLevel(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_level((d3d9_renderer::texture_handle_t)args[0], (int)args[1]);
    return 0;
}

static uint32_t h_GetTextureRotation(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_rotation((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTextureRotation(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_rotation((d3d9_renderer::texture_handle_t)args[0], arg_to_float(args[1]));
    return 0;
}

static uint32_t h_GetTexturePixelWidth(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_pixel_width((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_GetTexturePixelHeight(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) return float_to_arg(d3d9_renderer::get_texture_pixel_height((d3d9_renderer::texture_handle_t)args[0]));
    return float_to_arg(0.0f);
}

static uint32_t h_SetTexture(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) {
        const char* path = arg_to_string(args[1]);
        d3d9_renderer::set_texture((d3d9_renderer::texture_handle_t)args[0], path);
    }
    return 0;
}

static uint32_t h_SetTextureShow(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_show((d3d9_renderer::texture_handle_t)args[0], args[1] != 0);
    return 0;
}

static uint32_t h_SetTextureTrigger(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_texture_trigger((d3d9_renderer::texture_handle_t)args[0], args[1] != 0);
    return 0;
}

static uint32_t h_SetTextureSrcRect(const uint32_t* args, size_t nargs) {
    // Spec: (IHrect;)V — the rect is passed as a handle, we need to extract left/bottom/right/top
    // For now, use simplified 4-float version
    if (nargs >= 5) {
        d3d9_renderer::set_texture_src_rect(
            (d3d9_renderer::texture_handle_t)args[0],
            arg_to_float(args[1]), arg_to_float(args[2]),
            arg_to_float(args[3]), arg_to_float(args[4])
        );
    }
    return 0;
}

// ── Window/mouse/utility handlers ─────────────────────────────

static uint32_t h_GetMouseVectorX(const uint32_t* args, size_t nargs) {
    (void)args; (void)nargs;
    return float_to_arg(d3d9_renderer::get_mouse_vector_x());
}

static uint32_t h_GetMouseVectorY(const uint32_t* args, size_t nargs) {
    (void)args; (void)nargs;
    return float_to_arg(d3d9_renderer::get_mouse_vector_y());
}

static uint32_t h_GetWindowWidth(const uint32_t* args, size_t nargs) {
    (void)args; (void)nargs;
    return (uint32_t)d3d9_renderer::get_window_width();
}

static uint32_t h_GetWindowHeight(const uint32_t* args, size_t nargs) {
    (void)args; (void)nargs;
    return (uint32_t)d3d9_renderer::get_window_height();
}

static uint32_t h_ShowConsole(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) d3d9_renderer::show_console(args[0] != 0);
    return 0;
}

static uint32_t h_ShowFpsText(const uint32_t* args, size_t nargs) {
    if (nargs >= 1) d3d9_renderer::show_fps_text(args[0] != 0);
    return 0;
}

static uint32_t h_SetKeyboard(const uint32_t* args, size_t nargs) {
    if (nargs >= 2) d3d9_renderer::set_keyboard((int)args[0], args[1] != 0);
    return 0;
}

// ── Registration ──────────────────────────────────────────────

void register_all_handlers(register_handler_fn reg)
{
    if (!reg) return;

    // Font
    reg("CreateFont",        "()I",      h_CreateFont);
    reg("DestroyFont",       "(I)V",     h_DestroyFont);
    reg("GetFontWidth",      "(I)I",     h_GetFontWidth);
    reg("SetFontWidth",      "(II)V",    h_SetFontWidth);
    reg("GetFontHeight",     "(I)I",     h_GetFontHeight);
    reg("SetFontHeight",     "(II)V",    h_SetFontHeight);
    reg("GetFontWeight",     "(I)I",     h_GetFontWeight);
    reg("SetFontWeight",     "(II)V",    h_SetFontWeight);
    reg("SetFontItalic",     "(IB)V",    h_SetFontItalic);
    reg("GetFontFaceName",   "(I)S",     h_GetFontFaceName);
    reg("SetFontFaceName",   "(IS)V",    h_SetFontFaceName);

    // Text
    reg("CreateText",        "(ISRRRI)I", h_CreateText);
    reg("GetTextString",     "(I)S",     h_GetTextString);
    reg("SetTextString",     "(IS)V",    h_SetTextString);
    reg("GetTextX",          "(I)R",     h_GetTextX);
    reg("SetTextX",          "(IR)V",    h_SetTextX);
    reg("GetTextY",          "(I)R",     h_GetTextY);
    reg("SetTextY",          "(IR)V",    h_SetTextY);
    reg("GetTextTime",       "(I)R",     h_GetTextTime);
    reg("SetTextTime",       "(IR)V",    h_SetTextTime);
    reg("GetTextColor",      "(I)I",     h_GetTextColor);
    reg("SetTextColor",      "(II)V",    h_SetTextColor);

    // Texture
    reg("CreateTexture",     "(SRRRRII)I", h_CreateTexture);
    reg("DestroyTexture",    "(I)V",     h_DestroyTexture);
    reg("GetTextureX",       "(I)R",     h_GetTextureX);
    reg("SetTextureX",       "(IR)V",    h_SetTextureX);
    reg("GetTextureY",       "(I)R",     h_GetTextureY);
    reg("SetTextureY",       "(IR)V",    h_SetTextureY);
    reg("GetTextureWidth",   "(I)R",     h_GetTextureWidth);
    reg("SetTextureWidth",   "(IR)V",    h_SetTextureWidth);
    reg("GetTextureHeight",  "(I)R",     h_GetTextureHeight);
    reg("SetTextureHeight",  "(IR)V",    h_SetTextureHeight);
    reg("GetTextureColor",   "(I)I",     h_GetTextureColor);
    reg("SetTextureColor",   "(II)V",    h_SetTextureColor);
    reg("GetTextureLevel",   "(I)I",     h_GetTextureLevel);
    reg("SetTextureLevel",   "(II)V",    h_SetTextureLevel);
    reg("GetTextureRotation","(I)R",     h_GetTextureRotation);
    reg("SetTextureRotation","(IR)V",    h_SetTextureRotation);
    reg("GetTexturePixelWidth",  "(I)R", h_GetTexturePixelWidth);
    reg("GetTexturePixelHeight", "(I)R", h_GetTexturePixelHeight);
    reg("SetTexture",        "(IS)V",    h_SetTexture);
    reg("SetTextureShow",    "(IB)V",    h_SetTextureShow);
    reg("SetTextureTrigger", "(IB)V",    h_SetTextureTrigger);
    reg("SetTextureSrcRect", "(Iffff)V", h_SetTextureSrcRect);

    // Window/mouse/utility
    reg("GetMouseVectorX",   "()R",      h_GetMouseVectorX);
    reg("GetMouseVectorY",   "()R",      h_GetMouseVectorY);
    reg("GetWindowWidth",    "()I",      h_GetWindowWidth);
    reg("GetWindowHeight",   "()I",      h_GetWindowHeight);
    reg("ShowConsole",       "(B)V",     h_ShowConsole);
    reg("ShowFpsText",       "(B)V",     h_ShowFpsText);
    reg("SetKeyboard",       "(IB)V",    h_SetKeyboard);
}

// Set the jass::from_string function pointer (called from InitializeD3DAPI)
void set_from_string_fn(from_string_fn fn) {
    g_from_string = fn;
}

}  // namespace d3d9_api
