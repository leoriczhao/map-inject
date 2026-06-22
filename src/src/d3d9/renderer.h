// renderer.h — Text and texture rendering system for D3D9 overlay
//
// Called from CDirect3DDevice9::EndScene(). Manages:
//   - Font objects: GDI LOGFONT wrappers
//   - Text objects: screen-space text with color and lifetime
//   - Texture objects: D3D9 textures with position, size, color, rotation, z-order
//
// All coordinates are screen-space pixels (0,0 = top-left of War3 window).
// Colors are ARGB format (0xAARRGGBB).

#pragma once

#include <windows.h>
#include <d3d9.h>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace d3d9_renderer {

// ── Handle types ──────────────────────────────────────────────
typedef uint32_t font_handle_t;
typedef uint32_t text_handle_t;
typedef uint32_t texture_handle_t;

// ── Font object ───────────────────────────────────────────────
struct FontObj {
    int         height    = 16;
    int         width     = 0;       // 0 = auto
    int         weight    = FW_NORMAL;
    bool        italic    = false;
    std::string faceName  = "Arial";
    HFONT       hFont     = NULL;    // Created lazily, invalidated on change

    HFONT get_handle();
    void  invalidate();
    void  destroy();
};

// ── Text object ───────────────────────────────────────────────
struct TextObj {
    std::string  text;
    float        x         = 0.0f;
    float        y         = 0.0f;
    float        remaining = 0.0f;   // seconds remaining (-1 = infinite)
    uint32_t     color     = 0xFFFFFFFF;  // ARGB
    font_handle_t font     = 0;
    bool         visible   = true;
};

// ── Texture object ────────────────────────────────────────────
struct TextureObj {
    std::string          path;
    IDirect3DTexture9*   texture    = nullptr;
    float                x          = 0.0f;
    float                y          = 0.0f;
    float                width      = 0.0f;
    float                height     = 0.0f;
    uint32_t             color      = 0xFFFFFFFF;  // ARGB
    int                  level      = 0;           // z-order (higher = on top)
    float                rotation   = 0.0f;        // degrees
    bool                 visible    = true;
    bool                 triggerEnabled = true;

    // Source rect for texture cropping (in pixels, 0 = full texture)
    float                srcLeft    = 0.0f;
    float                srcTop     = 0.0f;
    float                srcRight   = 0.0f;  // 0 = use texture width
    float                srcBottom  = 0.0f;  // 0 = use texture height

    // Pixel dimensions of the loaded texture
    int                  pixelWidth  = 0;
    int                  pixelHeight = 0;
};

// ── Lifecycle (called from CDirect3DDevice9) ──────────────────
void on_device_created(IDirect3DDevice9* pDevice);
void on_device_lost();
void on_device_reset();
void on_end_scene(IDirect3DDevice9* pDevice);

// ── Font API ──────────────────────────────────────────────────
font_handle_t create_font();
void          destroy_font(font_handle_t h);
int           get_font_height(font_handle_t h);
void          set_font_height(font_handle_t h, int value);
int           get_font_width(font_handle_t h);
void          set_font_width(font_handle_t h, int value);
int           get_font_weight(font_handle_t h);
void          set_font_weight(font_handle_t h, int value);
void          set_font_italic(font_handle_t h, bool value);
const char*   get_font_facename(font_handle_t h);
void          set_font_facename(font_handle_t h, const char* value);

// ── Text API ──────────────────────────────────────────────────
text_handle_t create_text(font_handle_t font, const char* str, float x, float y, float time, uint32_t color);
void          destroy_text(text_handle_t h);
const char*   get_text_string(text_handle_t h);
void          set_text_string(text_handle_t h, const char* str);
float         get_text_x(text_handle_t h);
void          set_text_x(text_handle_t h, float value);
float         get_text_y(text_handle_t h);
void          set_text_y(text_handle_t h, float value);
float         get_text_time(text_handle_t h);
void          set_text_time(text_handle_t h, float value);
uint32_t      get_text_color(text_handle_t h);
void          set_text_color(text_handle_t h, uint32_t value);

// ── Texture API ───────────────────────────────────────────────
texture_handle_t create_texture(const char* path, float x, float y, float width, float height, uint32_t color, int level);
void             destroy_texture(texture_handle_t h);
float            get_texture_x(texture_handle_t h);
void             set_texture_x(texture_handle_t h, float value);
float            get_texture_y(texture_handle_t h);
void             set_texture_y(texture_handle_t h, float value);
float            get_texture_width(texture_handle_t h);
void             set_texture_width(texture_handle_t h, float value);
float            get_texture_height(texture_handle_t h);
void             set_texture_height(texture_handle_t h, float value);
uint32_t         get_texture_color(texture_handle_t h);
void             set_texture_color(texture_handle_t h, uint32_t value);
int              get_texture_level(texture_handle_t h);
void             set_texture_level(texture_handle_t h, int value);
float            get_texture_rotation(texture_handle_t h);
void             set_texture_rotation(texture_handle_t h, float value);
float            get_texture_pixel_width(texture_handle_t h);
float            get_texture_pixel_height(texture_handle_t h);
void             set_texture(texture_handle_t h, const char* path);
void             set_texture_show(texture_handle_t h, bool value);
void             set_texture_trigger(texture_handle_t h, bool value);

// Source rect (texture cropping)
// Returns: left, top, right, bottom via output params
void             get_texture_src_rect(texture_handle_t h, float* left, float* top, float* right, float* bottom);
void             set_texture_src_rect(texture_handle_t h, float left, float top, float right, float bottom);

// ── Window/mouse utilities ────────────────────────────────────
float get_mouse_vector_x();
float get_mouse_vector_y();
int   get_window_width();
int   get_window_height();

// ── Console/FPS ───────────────────────────────────────────────
void show_console(bool show);
void show_fps_text(bool show);

// ── Keyboard simulation ──────────────────────────────────────
void set_keyboard(int key, bool down);

}  // namespace d3d9_renderer
