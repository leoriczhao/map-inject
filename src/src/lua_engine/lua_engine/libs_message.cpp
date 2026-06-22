#include <lua.hpp>
#include <warcraft3/war3_searcher.h>
#include <warcraft3/version.h>
#include <warcraft3/keyboard_code.h>
#include <warcraft3/player.h>
#include <warcraft3/message_dispatch.h>
#include <warcraft3/jass/trampoline_function.h>
#include <base/hook/inline.h>
#include <base/hook/fp_call.h>
#include "common.h"
#include "callback.h"
#include "libs_runtime.h"
#include "jassbind.h"
#include <vector>
#include <map>

namespace warcraft3::lua_engine::message {

	// ============================================================
	// WndProc hook state
	// ============================================================
	static WNDPROC g_orig_wndproc = nullptr;
	static bool    g_wndproc_hooked = false;

	// ============================================================
	// Texture event system
	// ============================================================
	struct texture_entry {
		int      id;
		float    x, y, w, h;     // screen-space bounds (normalized 0..1 or pixel)
		bool     visible;
		bool     trigger_enabled;
	};

	struct texture_event_entry {
		int      event_id;
		int      texture_id;
		int      keycode;          // keyboard key (WARK_*), or -1 for mouse
		uint32_t press_lua_ref;    // Lua registry ref for press callback
		uint32_t release_lua_ref;  // Lua registry ref for release callback
	};

	static int g_next_texture_id = 1;
	static int g_next_event_id   = 1;
	static std::map<int, texture_entry>       g_textures;
	static std::map<int, texture_event_entry> g_texture_events;

	// ============================================================
	// Convert WARK_* code to Windows VK code for simulation
	// ============================================================
	static int wark_to_vk(int wark)
	{
		// Direct ASCII matches (A-Z, 0-9, space)
		if (wark == WARK_SPACE)         return VK_SPACE;
		if (wark >= WARK_0 && wark <= WARK_9)     return wark;   // 0x30..0x39
		if (wark >= WARK_A && wark <= WARK_Z)     return wark;   // 0x41..0x5A
		if (wark == WARK_SHIFT)         return VK_SHIFT;
		if (wark == WARK_CTRL)          return VK_CONTROL;
		if (wark == WARK_ALT)           return VK_MENU;

		// Numpad
		if (wark >= WARK_NUMPAD0 && wark <= WARK_NUMPAD9) return VK_NUMPAD0 + (wark - WARK_NUMPAD0);
		if (wark == WARK_ADD)           return VK_ADD;
		if (wark == WARK_SUBTRACT)      return VK_SUBTRACT;
		if (wark == WARK_MULTIPLY)      return VK_MULTIPLY;
		if (wark == WARK_DIVIDE)        return VK_DIVIDE;
		if (wark == WARK_DECIMAL)       return VK_DECIMAL;

		// OEM keys
		if (wark == WARK_TILDE)         return VK_OEM_3;
		if (wark == WARK_OEM_PLUS)      return VK_OEM_PLUS;
		if (wark == WARK_OEM_MINUS)     return VK_OEM_MINUS;
		if (wark == WARK_OEM_OBRACKE)   return VK_OEM_4;
		if (wark == WARK_OEM_CBRACKE)   return VK_OEM_6;
		if (wark == WARK_OEM_BACKSLASH) return VK_OEM_5;
		if (wark == WARK_OEM_SEMICOLON) return VK_OEM_1;
		if (wark == WARK_OEM_SQUOTMARKS)return VK_OEM_7;
		if (wark == WARK_OEM_COMMA)     return VK_OEM_COMMA;
		if (wark == WARK_OEM_PERIOD)    return VK_OEM_PERIOD;
		if (wark == WARK_OEM_SLASH)     return VK_OEM_2;

		// Special keys
		if (wark == WARK_ESC)           return VK_ESCAPE;
		if (wark == WARK_ENTER)         return VK_RETURN;
		if (wark == WARK_BACKSPACE)     return VK_BACK;
		if (wark == WARK_TAB)           return VK_TAB;
		if (wark == WARK_RIGHT)         return VK_RIGHT;
		if (wark == WARK_UP)            return VK_UP;
		if (wark == WARK_LEFT)          return VK_LEFT;
		if (wark == WARK_DOWN)          return VK_DOWN;
		if (wark == WARK_INSERT)        return VK_INSERT;
		if (wark == WARK_DELETE)        return VK_DELETE;
		if (wark == WARK_HOME)          return VK_HOME;
		if (wark == WARK_END)           return VK_END;
		if (wark == WARK_PAGEUP)        return VK_PRIOR;
		if (wark == WARK_PAGEDOWN)      return VK_NEXT;
		if (wark == WARK_CAPSLOCK)      return VK_CAPITAL;
		if (wark == WARK_NUMLOCK)       return VK_NUMLOCK;
		if (wark == WARK_SCROLLLOCK)    return VK_SCROLL;
		if (wark == WARK_PAUSE)         return VK_PAUSE;
		if (wark == WARK_PRINTSCREEN)   return VK_SNAPSHOT;

		// F-keys
		if (wark >= WARK_F1 && wark <= WARK_F12) return VK_F1 + (wark - WARK_F1);

		return 0;
	}

	static HWND FindWar3Window()
	{
		DWORD current_pid = GetCurrentProcessId();
		HWND hwnd = NULL;
		while (NULL != (hwnd = FindWindowExW(NULL, hwnd, L"Warcraft III", L"Warcraft III")))
		{
			DWORD pid = 0;
			GetWindowThreadProcessId(hwnd, &pid);
			if (pid == current_pid)
			{
				return hwnd;
			}
		}
		return NULL;
	}

	static HWND get_window()
	{
		static HWND war3_window =  FindWar3Window();
		return war3_window;
	}

	struct AbilityData
	{
		uintptr_t  vft_ptr;
		uintptr_t  ability_id;
		uintptr_t  order_id;
		uintptr_t  unk;
		uintptr_t  target_type;
	};

	struct CCommandButton
	{
		uintptr_t    vft_ptr;
		uintptr_t    unk[0x63];
		AbilityData* ability;   // offset 0x190
	};

	struct message_t
	{
		uint32_t vfptr;
		uint32_t unk01;
		uint32_t msgid;
	};

	struct keyboard_message_t
		: public message_t
	{
		uint32_t unk03;
		uint32_t keycode;
		uint8_t  keystate;
	};

	struct mouse_message_t
		: public message_t
	{
		uint32_t unk03;
		uint32_t unk04;
		float rx;
		float ry;
		float wx;
		float wy;
		uint32_t unk0A;
		uint32_t code;
	};

	struct ability_mouse_message_t
		: public message_t
	{
		uint32_t unk03;
		uint32_t unk04;
		uint32_t unk05;
		uint32_t unk06;
		uint32_t unk07;
		uint32_t unk08;
		uint32_t code;
		uint32_t unk0A;
		uint32_t unk0B;
		CCommandButton* button;
	};

	static uintptr_t search_convert_xy()
	{
		war3_searcher& s = get_war3_searcher();
		uintptr_t ptr = 0;
		if (s.get_version() <= version_120e)
		{
			ptr = s.search_string("E:\\Drive1\\temp\\buildwar3x\\War3\\Source\\UI\\CWorldFrameWar3.cpp");
			ptr += 4;
			ptr = next_opcode(ptr, 0xC7, 6);
			ptr += 6;
			ptr = next_opcode(ptr, 0xC7, 6);
			ptr += 6;
			ptr = next_opcode(ptr, 0xC7, 6);
			ptr += 2;
			ptr = *(uintptr_t*)ptr;
		}
		else
		{
			ptr = get_vfn_ptr(".?AVCWorldFrameWar3@@");
		}
		ptr += 0x38;
		ptr = *(uintptr_t*)ptr;
		do
		{
			ptr = next_opcode(ptr, 0x68, 5);
			ptr += 5;
		} while (*(uintptr_t*)(ptr - 4) != 0x1A0068);
		ptr = next_opcode(ptr, 0xE8, 5);
		ptr += 5;
		ptr = next_opcode(ptr, 0xE8, 5);
		if (s.get_version() >= version_127a)
		{
			ptr += 5;
			ptr = next_opcode(ptr, 0xE8, 5);
			ptr += 5;
			ptr = next_opcode(ptr, 0xE8, 5);
		}
		ptr = convert_function(ptr);
		return ptr;
	}

	static int convert_xy(float rx, float ry, float& wx, float& wy)
	{
		static uintptr_t s_convert_xy = search_convert_xy();
		war3_searcher& s = get_war3_searcher();
		uint32_t cgameui = s.get_gameui(0, 0);
		float buf[0x10] = { 0 };
		uintptr_t cworldframe = (s.get_version() <= version_120e) ? *(uintptr_t*)(cgameui + 0x3B0) : *(uintptr_t*)(cgameui + 0x3BC);
		int result = base::this_call<int>(s_convert_xy, cworldframe, rx, ry, buf, 1, 1);
		wx = buf[4];
		wy = buf[5];
		return result;
	}

	static int lmouse(lua_State* L)
	{
		HWND war3_window = get_window();
		POINT pt = { 0 };
		::GetCursorPos(&pt);
		ScreenToClient(war3_window, &pt);
		RECT rt;
		::GetClientRect(war3_window, &rt);
		float fx = 0.8f *((float)(pt.x - rt.left) / (rt.right - rt.left));
		float fy = 0.6f *(1.0f - (float)(pt.y - rt.top) / (rt.bottom - rt.top));
		float wx = 0.f, wy = 0.f;
		convert_xy(fx, fy, wx, wy);
		lua_pushnumber(L, wx);
		lua_pushnumber(L, wy);
		return 2;
	}

	static int lbutton(lua_State* L)
	{
		int x = (int)lua_tointeger(L, 1);
		int y = (int)lua_tointeger(L, 2);
		if (x < 0 || x >= 4 || y < 0 || y >= 3)
		{
			return 0;
		}

		war3_searcher& s = get_war3_searcher();
		uint32_t cgameui = s.get_gameui(0, 0);
		uintptr_t button_bar = (s.get_version() <= version_120e) ? *(uintptr_t*)(cgameui + 0x3BC) : *(uintptr_t*)(cgameui + 0x3C8);
		uintptr_t button_array = *(uintptr_t*)(button_bar + 0x154);
		CCommandButton* button = ((CCommandButton*)*(uintptr_t*)(*(uintptr_t*)(button_array + 0x10 * (y)+0x08) + 0x04 * (x)));
		AbilityData* ability = button->ability;
		if (!ability)
		{
			return 0;
		}
		lua_pushinteger(L, ability->ability_id);
		lua_pushinteger(L, ability->order_id);
		lua_pushinteger(L, ability->target_type & 0x3FF);
		return 3;
	}
	
	static uintptr_t get_select_unit()
	{
		player::selection_t* slt = player::selection(player::local());
		if (!slt || !slt->unit)
		{
			return 0;
		}
		return slt->unit;
	}

	static int lselection(lua_State* L)
	{
		uintptr_t unit = get_select_unit();
		if (!unit)
		{
			jassbind::push_handle(L, 0);
			return 1;
		}
		uintptr_t handle = object_to_handle(unit);
		jassbind::push_handle(L, handle);
		return 1;
	}

	namespace order {
		bool b_hook = false;
		bool b_search = false;

		namespace real {
			uintptr_t immediate_order = 0;
			uintptr_t point_order = 0;
			uintptr_t target_order = 0;
		}

		namespace fake {
			int __fastcall immediate_order(uint32_t order, uint32_t unk, uint32_t flags)
			{
				if (order >= 'A000')
				{
					printf("immediate_order, %c%c%c%c, %X, %X\n", ((char*)&order)[3], ((char*)&order)[2], ((char*)&order)[1], ((char*)&order)[0], unk, flags);
				}
				else
				{
					printf("immediate_order, %X, %X, %X\n", order, unk, flags);
				}
				return base::fast_call<int>(real::immediate_order, order, unk, flags);
			}

			int __fastcall point_order(uint32_t order, uint32_t unk, float* x, float* y, uint32_t flags)
			{
				if (order >= 'A000')
				{
					printf("point_order, %c%c%c%c, %X, %f, %f, %X\n", ((char*)&order)[3], ((char*)&order)[2], ((char*)&order)[1], ((char*)&order)[0], unk, *x, *y, flags);
				}
				else
				{
					printf("point_order, %X, %X, %f, %f, %X\n", order, unk, *x, *y, flags);
				}
				return base::fast_call<int>(real::point_order, order, unk, x, y, flags);
			}

			int __fastcall target_order(uint32_t order, uint32_t unk, float* x, float* y, uint32_t target, uint32_t flags)
			{
				if (order >= 'A000')
				{
					printf("target_order, %c%c%c%c, %X, %f, %f, %X, %X\n", ((char*)&order)[3], ((char*)&order)[2], ((char*)&order)[1], ((char*)&order)[0], unk, *x, *y, target, flags);
				}
				else
				{
					printf("target_order, %X, %X, %f, %f, %X, %X\n", order, unk, *x, *y, target, flags);
				}
				return base::fast_call<int>(real::target_order, order, unk, x, y, target, flags);
			}
		}

		static void search()
		{
			if (b_search) {
				return;
			}
			b_search = true;

			war3_searcher& s = get_war3_searcher();
			uintptr_t ptr = 0;
			if (s.get_version() >= version_127a)
			{
				ptr = s.base() + 0x3AE4E0;
			}
			else
			{
				ptr = s.search_string("SimpleDestructableNameValue");
				ptr += 4;
				do
				{
					ptr = next_opcode(ptr, 0xC1, 3);
					ptr += 3;
				} while (*(uint16_t*)(ptr - 2) != 0x05E2);
			}

			ptr = next_opcode(ptr, 0xE8, 5);
			real::immediate_order = convert_function(ptr);
			ptr += 5;
			ptr = next_opcode(ptr, 0xE8, 5);
			real::point_order = convert_function(ptr);
			ptr += 5;
			ptr = next_opcode(ptr, 0xE8, 5);
			ptr += 5;
			ptr = next_opcode(ptr, 0xE8, 5);
			real::target_order = convert_function(ptr);
		}

		static int limmediate(lua_State* L)
		{
			if (!get_select_unit()){
				lua_pushboolean(L, 0);
				return 1;
			}
			search();
			uint32_t order = lua_tointeger(L, 1);
			uint32_t flags = lua_tointeger(L, 2);
            base::fast_call<int>(real::immediate_order, order, 0, flags);
			lua_pushboolean(L, 1);
			return 1;
		}

		static int lpoint(lua_State* L)
		{
			if (!get_select_unit()){
				lua_pushboolean(L, 0);
				return 1;
			}
			search();
			uint32_t order = lua_tointeger(L, 1);
			float x = (float)lua_tonumber(L, 2);
			float y = (float)lua_tonumber(L, 3);
			uint32_t flags = lua_tointeger(L, 4);
            base::fast_call<int>(real::point_order, order, 0, &x, &y, flags);
			lua_pushboolean(L, 1);
			return 1;
		}

		static int ltarget(lua_State* L)
		{
			if (!get_select_unit()){
				lua_pushboolean(L, 0);
				return 1;
			}
			search();
			uint32_t order = lua_tointeger(L, 1);
			float x = (float)lua_tonumber(L, 2);
			float y = (float)lua_tonumber(L, 3);
			uint32_t target = jassbind::read_handle(L, 4);
			uint32_t flags = lua_tointeger(L, 5);
			if (target)
			{
				target = handle_to_object(target);
			}
            base::fast_call<int>(real::target_order, order, 0, &x, &y, target, flags);
			lua_pushboolean(L, 1);
			return 1;
		}

		static int lenable_debug(lua_State* /*L*/)
		{
			if (b_hook) {
				return 0;
			}
			b_hook = true;
			search();

            base::hook::install(&real::immediate_order, (uintptr_t)fake::immediate_order);
            base::hook::install(&real::point_order, (uintptr_t)fake::point_order);
            base::hook::install(&real::target_order, (uintptr_t)fake::target_order);
			return 0;
		}
	}

	// ============================================================
	// Forward declaration: ML is the main Lua state, set in lset()
	// when the message hook is installed.
	// ============================================================
	static lua_State* ML = 0;

	// ============================================================
	// Texture event dispatch: check if a WndProc message matches
	// any registered texture event's key and mouse-in-bounds
	// ============================================================
	static void dispatch_texture_events(lua_State* L, UINT msg_type, int wark_key, bool is_press, int mx, int my)
	{
		if (!L) return;
		for (auto& kv : g_texture_events)
		{
			const texture_event_entry& ev = kv.second;
			// Match key: -1 means mouse event (button), otherwise keyboard
			if (ev.keycode != wark_key) continue;

			// Look up the texture
			auto it = g_textures.find(ev.texture_id);
			if (it == g_textures.end()) continue;
			const texture_entry& tex = it->second;
			if (!tex.visible || !tex.trigger_enabled) continue;

			// Check mouse bounds (tex coords are in screen-space pixels from GetCursorPos)
			// mx, my are in client coords
			HWND hw = get_window();
			if (!hw) continue;
			RECT rt;
			GetClientRect(hw, &rt);
			float norm_x = (float)mx / (float)(rt.right - rt.left);
			float norm_y = (float)my / (float)(rt.bottom - rt.top);
			if (norm_x < tex.x || norm_x > tex.x + tex.w) continue;
			if (norm_y < tex.y || norm_y > tex.y + tex.h) continue;

			// Fire the callback via Lua registry ref
			uint32_t lua_ref = is_press ? ev.press_lua_ref : ev.release_lua_ref;
			if (lua_ref == 0) continue;
			safe_call_ref(L, lua_ref, 0, jass::TYPE_BOOLEAN);
		}
	}

	// ============================================================
	// WndProc hook: intercept raw Windows messages before War3
	// ============================================================
	static bool wndproc_handle_key(lua_State* L, UINT msg_type, WPARAM wParam, LPARAM lParam)
	{
		// Convert Windows VK code to WARK_* code
		int vk = (int)wParam;
		int wark = 0;

		// Reverse mapping: VK -> WARK
		if (vk == VK_SPACE)         wark = WARK_SPACE;
		else if (vk >= '0' && vk <= '9')     wark = vk;
		else if (vk >= 'A' && vk <= 'Z')     wark = vk;
		else if (vk == VK_SHIFT)    wark = WARK_SHIFT;
		else if (vk == VK_CONTROL)  wark = WARK_CTRL;
		else if (vk == VK_MENU)     wark = WARK_ALT;
		else if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) wark = WARK_NUMPAD0 + (vk - VK_NUMPAD0);
		else if (vk == VK_ADD)      wark = WARK_ADD;
		else if (vk == VK_SUBTRACT) wark = WARK_SUBTRACT;
		else if (vk == VK_MULTIPLY) wark = WARK_MULTIPLY;
		else if (vk == VK_DIVIDE)   wark = WARK_DIVIDE;
		else if (vk == VK_DECIMAL)  wark = WARK_DECIMAL;
		else if (vk == VK_OEM_3)    wark = WARK_TILDE;
		else if (vk == VK_OEM_PLUS) wark = WARK_OEM_PLUS;
		else if (vk == VK_OEM_MINUS)wark = WARK_OEM_MINUS;
		else if (vk == VK_OEM_4)    wark = WARK_OEM_OBRACKE;
		else if (vk == VK_OEM_6)    wark = WARK_OEM_CBRACKE;
		else if (vk == VK_OEM_5)    wark = WARK_OEM_BACKSLASH;
		else if (vk == VK_OEM_1)    wark = WARK_OEM_SEMICOLON;
		else if (vk == VK_OEM_7)    wark = WARK_OEM_SQUOTMARKS;
		else if (vk == VK_OEM_COMMA)wark = WARK_OEM_COMMA;
		else if (vk == VK_OEM_PERIOD)wark= WARK_OEM_PERIOD;
		else if (vk == VK_OEM_2)    wark = WARK_OEM_SLASH;
		else if (vk == VK_ESCAPE)   wark = WARK_ESC;
		else if (vk == VK_RETURN)   wark = WARK_ENTER;
		else if (vk == VK_BACK)     wark = WARK_BACKSPACE;
		else if (vk == VK_TAB)      wark = WARK_TAB;
		else if (vk == VK_RIGHT)    wark = WARK_RIGHT;
		else if (vk == VK_UP)       wark = WARK_UP;
		else if (vk == VK_LEFT)     wark = WARK_LEFT;
		else if (vk == VK_DOWN)     wark = WARK_DOWN;
		else if (vk == VK_INSERT)   wark = WARK_INSERT;
		else if (vk == VK_DELETE)   wark = WARK_DELETE;
		else if (vk == VK_HOME)     wark = WARK_HOME;
		else if (vk == VK_END)      wark = WARK_END;
		else if (vk == VK_PRIOR)    wark = WARK_PAGEUP;
		else if (vk == VK_NEXT)     wark = WARK_PAGEDOWN;
		else if (vk == VK_CAPITAL)  wark = WARK_CAPSLOCK;
		else if (vk == VK_NUMLOCK)  wark = WARK_NUMLOCK;
		else if (vk == VK_SCROLL)   wark = WARK_SCROLLLOCK;
		else if (vk == VK_PAUSE)    wark = WARK_PAUSE;
		else if (vk == VK_SNAPSHOT) wark = WARK_PRINTSCREEN;
		else if (vk >= VK_F1 && vk <= VK_F12) wark = WARK_F1 + (vk - VK_F1);

		bool is_press = (msg_type == WM_KEYDOWN || msg_type == WM_SYSKEYDOWN);
		POINT pt;
		GetCursorPos(&pt);
		HWND hw = get_window();
		if (hw) ScreenToClient(hw, &pt);
		dispatch_texture_events(L, msg_type, wark, is_press, pt.x, pt.y);
		return false;  // never consume the message here
	}

	static bool wndproc_handle_mouse(lua_State* L, UINT msg_type, WPARAM wParam, LPARAM lParam)
	{
		int mx = (short)LOWORD(lParam);
		int my = (short)HIWORD(lParam);
		int wark_key = -1;
		bool is_press = false;

		switch (msg_type)
		{
		case WM_LBUTTONDOWN: wark_key = 0; is_press = true;  break;
		case WM_LBUTTONUP:   wark_key = 0; is_press = false; break;
		case WM_RBUTTONDOWN: wark_key = 1; is_press = true;  break;
		case WM_RBUTTONUP:   wark_key = 1; is_press = false; break;
		default: return false;
		}

		dispatch_texture_events(L, msg_type, wark_key, is_press, mx, my);
		return false;  // never consume here
	}

	static LRESULT CALLBACK war3_wndproc_hook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		lua_State* L = ML;
		if (L)
		{
			switch (msg)
			{
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				wndproc_handle_key(L, msg, wParam, lParam);
				break;
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				wndproc_handle_mouse(L, msg, wParam, lParam);
				break;
			default:
				break;
			}
		}
		return CallWindowProcW(g_orig_wndproc, hWnd, msg, wParam, lParam);
	}

	static void install_wndproc_hook()
	{
		if (g_wndproc_hooked) return;
		HWND hw = get_window();
		if (!hw) return;
		g_orig_wndproc = (WNDPROC)SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)war3_wndproc_hook);
		g_wndproc_hooked = (g_orig_wndproc != nullptr);
	}

	static void remove_wndproc_hook()
	{
		if (!g_wndproc_hooked) return;
		HWND hw = get_window();
		if (hw && g_orig_wndproc)
		{
			SetWindowLongPtrW(hw, GWLP_WNDPROC, (LONG_PTR)g_orig_wndproc);
		}
		g_orig_wndproc = nullptr;
		g_wndproc_hooked = false;
	}

	// ============================================================
	// Existing CGameUI filter (preserved exactly as before)
	// ============================================================
	static bool keyboard_event(lua_State* L, const char* type, keyboard_message_t* msg)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "_JASS_MESSAGE_HANDLE");
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);
			return true;
		}
		lua_newtable(L);
		lua_pushstring(L, type);
		lua_setfield(L, -2, "type");
		if (msg)
		{
			lua_pushinteger(L, msg->keycode);
			lua_setfield(L, -2, "code");
			lua_pushinteger(L, msg->keystate);
			lua_setfield(L, -2, "state");
		}
		lua_insert(L, -2);
		lua_pushvalue(L, -2);
		if (safe_call_not_sleep(L, 1, 1, true) != LUA_OK)
		{
			lua_pop(L, 1);
			return true;
		}
		int ok = lua_toboolean(L, -1);
		lua_pop(L, 1);
		if (ok && msg)
		{
			lua_getfield(L, -1, "code");
			msg->keycode = lua_tointeger(L, -1);
			lua_getfield(L, -2, "state");
			msg->keystate = lua_tointeger(L, -1);
			lua_pop(L, 2);
		}
		lua_pop(L, 1);
		return !!ok;
	}

	static bool mouse_event(lua_State* L, const char* type, mouse_message_t* msg)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "_JASS_MESSAGE_HANDLE");
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);
			return true;
		}
		lua_newtable(L);
		lua_pushstring(L, type);
		lua_setfield(L, -2, "type");
		if (msg)
		{
			lua_pushinteger(L, msg->code);
			lua_setfield(L, -2, "code");
			lua_pushnumber(L, msg->wx);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, msg->wy);
			lua_setfield(L, -2, "y");
		}
		lua_insert(L, -2);
		lua_pushvalue(L, -2);
		if (safe_call_not_sleep(L, 1, 1, true) != LUA_OK)
		{
			lua_pop(L, 1);
			return true;
		}
		int ok = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_pop(L, 1);
		return !!ok;
	}

	static bool ability_mouse_event(lua_State* L, ability_mouse_message_t* msg)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "_JASS_MESSAGE_HANDLE");
		if (!lua_isfunction(L, -1))
		{
			lua_pop(L, 1);
			return true;
		}
		lua_newtable(L);
		lua_pushstring(L, "mouse_ability");
		lua_setfield(L, -2, "type");
		if (msg && msg->button&& msg->button->ability)
		{
			lua_pushinteger(L, msg->code);
			lua_setfield(L, -2, "code");
			lua_pushinteger(L, msg->button->ability->ability_id);
			lua_setfield(L, -2, "ability");
			lua_pushinteger(L, msg->button->ability->order_id);
			lua_setfield(L, -2, "order");
		}
		lua_insert(L, -2);
		lua_pushvalue(L, -2);
		if (safe_call_not_sleep(L, 1, 1, true) != LUA_OK)
		{
			lua_pop(L, 1);
			return true;
		}
		int ok = lua_toboolean(L, -1);
		lua_pop(L, 1);
		lua_pop(L, 1);
		return !!ok;
	}

	static bool __fastcall filter(uintptr_t /*cgameui*/, uintptr_t /*edx*/, message_t* msg)
	{
		switch (msg->msgid)
		{
		case 0x30064:
			return ability_mouse_event(ML, (ability_mouse_message_t*)msg);
			break;
		case 0x40060064:
			return keyboard_event(ML, "key_down", (keyboard_message_t*)msg);
		case 0x40060066:
			return keyboard_event(ML, "key_up", (keyboard_message_t*)msg);
			break;
		case 0x40060067:
			break;
		case 0x1A0068:
			return mouse_event(ML, "mouse_down", (mouse_message_t*)msg);
		case 0x400500C9:
			return mouse_event(ML, "mouse_up", 0);
		default:
			break;
		}
		return true;
	}

	static int lget(lua_State* L)
	{
		const char* name = lua_tostring(L, 2);
		if (strcmp("hook", name) == 0)
		{
			lua_getfield(L, LUA_REGISTRYINDEX, "_JASS_MESSAGE_HANDLE");
			return 1;
		}
		return 0;
	}

	static int lset(lua_State* L)
	{
		static bool hook = false;
		const char* name = lua_tostring(L, 2);
		if (strcmp("hook", name) == 0)
		{
			if (lua_isfunction(L, 3))
			{
				lua_pushvalue(L, 3);
				lua_setfield(L, LUA_REGISTRYINDEX, "_JASS_MESSAGE_HANDLE");
				ML = get_mainthread(L);
				if (!hook) {
					message_dispatch::add_filter((uintptr_t)filter);
					install_wndproc_hook();
					hook = !hook;
				}
			}
			else if (lua_isnil(L, 3))
			{
				if (hook) {
					message_dispatch::remove_filter((uintptr_t)filter);
					remove_wndproc_hook();
					hook = !hook;
				}
				lua_pushnil(L);
				lua_setfield(L, LUA_REGISTRYINDEX, "_JASS_MESSAGE_HANDLE");
			}
			return 0;
		}
		return 0;
	}

	static int init_keyboard(lua_State* L)
	{
		lua_newtable(L);
#define SET_KEYBOARD(name) \
		lua_pushinteger(L, WARK_ ## name);	\
		lua_setfield(L, -2, #name)
		SET_KEYBOARD_ALL();
#undef SET_KEYBOARD
		return 1;
	}

	// ============================================================
	// Screen-space mouse coordinates (GetMouseVectorX / GetMouseVectorY)
	// Returns mouse position relative to War3 window in normalized [0..1] range
	// matching builtin-japi's GetMouseVectorX/GetMouseVectorY semantics
	// ============================================================
	static int lmouse_screen(lua_State* L)
	{
		HWND hw = get_window();
		POINT pt = { 0 };
		GetCursorPos(&pt);
		if (hw) ScreenToClient(hw, &pt);
		RECT rt = { 0 };
		if (hw) GetClientRect(hw, &rt);
		int w = rt.right - rt.left;
		int h = rt.bottom - rt.top;
		if (w <= 0) w = 1;
		if (h <= 0) h = 1;
		// Return normalized coordinates (0..1), matching builtin-japi semantics
		lua_pushnumber(L, (double)pt.x / (double)w);
		lua_pushnumber(L, (double)pt.y / (double)h);
		return 2;
	}

	// ============================================================
	// Keyboard simulation
	// SetKeyboard(key, is_down): set a key's state (press or release)
	// Uses PostMessage to the War3 window so it arrives through the
	// normal message queue (respects War3's internal dispatch).
	// ============================================================
	static int lset_keyboard(lua_State* L)
	{
		int wark_key = (int)lua_tointeger(L, 1);
		bool is_down = lua_toboolean(L, 2) != 0;

		int vk = wark_to_vk(wark_key);
		if (vk == 0)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		HWND hw = get_window();
		if (!hw)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		UINT msg = is_down ? WM_KEYDOWN : WM_KEYUP;
		LPARAM lParam = 0x00000001;  // repeat count=1, scan code=0
		if (!is_down)
		{
			lParam |= (0xC0 << 24);  // set previous key state + transition state bits
		}

		PostMessageW(hw, msg, (WPARAM)vk, lParam);
		lua_pushboolean(L, 1);
		return 1;
	}

	// ClickKeyboard(key): simulate a full press+release cycle
	static int lclick_keyboard(lua_State* L)
	{
		int wark_key = (int)lua_tointeger(L, 1);
		int vk = wark_to_vk(wark_key);
		if (vk == 0) return 0;

		HWND hw = get_window();
		if (!hw) return 0;

		// Press
		LPARAM lParamDown = 0x00000001;
		PostMessageW(hw, WM_KEYDOWN, (WPARAM)vk, lParamDown);
		// Release
		LPARAM lParamUp = 0x00000001 | (0xC0 << 24);
		PostMessageW(hw, WM_KEYUP, (WPARAM)vk, lParamUp);
		return 0;
	}

	// ============================================================
	// ShowConsole: show/hide the War3 console UI
	// builtin-japi sets a flag that hides the command card / console
	// We implement it by finding the game UI frame and hiding child
	// windows, or by calling the War3 internal function.
	// Simplified: toggle console window visibility.
	// ============================================================
	static int lshow_console(lua_State* L)
	{
		bool show = lua_toboolean(L, 1) != 0;
		// The War3 console is rendered by the game engine, not a Win32 window.
		// In builtin-japi, ShowConsole hides the bottom console UI (command card,
		// resource bar, etc.). We find the ConsoleFrame via CGameUI child lookup.
		// For now, use the simpler approach: find "ConsoleWindowClass" child.
		HWND hw = get_window();
		if (!hw)
		{
			lua_pushboolean(L, 0);
			return 1;
		}

		// Try to find the console child window. War3 may use different class names.
		// Also try generic approach: find all child windows and toggle visibility.
		// builtin-japi actually manipulates the Frame (CSimpleFontString / CLayoutFrame),
		// not a Win32 HWND. We approximate with ShowWindow on the main window.
		// A proper implementation would call CGameUI->SetConsoleVisible().

		// Search for child windows with common War3 console class names
		static const wchar_t* console_classes[] = {
			L"ConsoleWindowClass",
			L"Warcraft III",
		};

		bool found = false;
		for (auto cls : console_classes)
		{
			HWND child = FindWindowExW(hw, NULL, cls, NULL);
			if (child)
			{
				ShowWindow(child, show ? SW_SHOW : SW_HIDE);
				found = true;
			}
		}

		// If no specific console window found, we can still toggle the
		// entire game's console rendering via the CGameUI pointer.
		// For now, store the state for future use.
		lua_pushboolean(L, found ? 1 : 0);
		return 1;
	}

	// ============================================================
	// TextureAddEvent: bind an event to a screen-space texture region
	// Lua API:
	//   local event_id = message.texture_add_event(texture_id, keycode, press_func, release_func)
	//   message.texture_remove_event(texture_id, event_id)
	//
	// texture_id: from message.texture_create()
	// keycode: WARK_* value from message.keyboard, or -1 for mouse buttons
	// press_func / release_func: Lua functions or JASS code references
	//
	// When a WndProc message matches the keycode AND the mouse is within
	// the texture's screen bounds, the corresponding callback fires.
	// ============================================================
	static int ltexture_create(lua_State* L)
	{
		// Args: x, y, w, h (normalized 0..1 screen coords)
		float x = (float)lua_tonumber(L, 1);
		float y = (float)lua_tonumber(L, 2);
		float w = (float)lua_tonumber(L, 3);
		float h = (float)lua_tonumber(L, 4);

		int id = g_next_texture_id++;
		texture_entry tex;
		tex.id = id;
		tex.x = x;
		tex.y = y;
		tex.w = w;
		tex.h = h;
		tex.visible = true;
		tex.trigger_enabled = true;
		g_textures[id] = tex;

		lua_pushinteger(L, id);
		return 1;
	}

	static int ltexture_destroy(lua_State* L)
	{
		int tex_id = (int)lua_tointeger(L, 1);
		g_textures.erase(tex_id);
		// Remove all events for this texture
		std::vector<int> to_remove;
		for (auto& kv : g_texture_events)
		{
			if (kv.second.texture_id == tex_id)
				to_remove.push_back(kv.first);
		}
		for (int id : to_remove) g_texture_events.erase(id);
		return 0;
	}

	static int ltexture_set_visible(lua_State* L)
	{
		int tex_id = (int)lua_tointeger(L, 1);
		bool vis = lua_toboolean(L, 2) != 0;
		auto it = g_textures.find(tex_id);
		if (it != g_textures.end()) it->second.visible = vis;
		return 0;
	}

	static int ltexture_set_trigger(lua_State* L)
	{
		int tex_id = (int)lua_tointeger(L, 1);
		bool en = lua_toboolean(L, 2) != 0;
		auto it = g_textures.find(tex_id);
		if (it != g_textures.end()) it->second.trigger_enabled = en;
		return 0;
	}

	static int ltexture_set_rect(lua_State* L)
	{
		int tex_id = (int)lua_tointeger(L, 1);
		float x = (float)lua_tonumber(L, 2);
		float y = (float)lua_tonumber(L, 3);
		float w = (float)lua_tonumber(L, 4);
		float h = (float)lua_tonumber(L, 5);
		auto it = g_textures.find(tex_id);
		if (it != g_textures.end())
		{
			it->second.x = x;
			it->second.y = y;
			it->second.w = w;
			it->second.h = h;
		}
		return 0;
	}

	static int ltexture_add_event(lua_State* L)
	{
		// Args: texture_id, keycode, press_func, release_func
		int tex_id   = (int)lua_tointeger(L, 1);
		int keycode  = (int)lua_tointeger(L, 2);

		// Validate texture exists
		if (g_textures.find(tex_id) == g_textures.end())
		{
			lua_pushinteger(L, 0);
			return 1;
		}

		int ev_id = g_next_event_id++;
		texture_event_entry ev;
		ev.event_id         = ev_id;
		ev.texture_id       = tex_id;
		ev.keycode          = keycode;
		ev.press_lua_ref    = 0;
		ev.release_lua_ref  = 0;

		// Arg 3: press callback (Lua function)
		if (lua_isfunction(L, 3))
		{
			// Store Lua ref for direct callback from WndProc
			ev.press_lua_ref = runtime::callback_push(L, 3);
		}
		else if (lua_isnumber(L, 3))
		{
			// Already a Lua registry ref (integer)
			ev.press_lua_ref = (uint32_t)lua_tointeger(L, 3);
		}

		// Arg 4: release callback (Lua function)
		if (lua_isfunction(L, 4))
		{
			ev.release_lua_ref = runtime::callback_push(L, 4);
		}
		else if (lua_isnumber(L, 4))
		{
			ev.release_lua_ref = (uint32_t)lua_tointeger(L, 4);
		}

		g_texture_events[ev_id] = ev;
		lua_pushinteger(L, ev_id);
		return 1;
	}

	static int ltexture_remove_event(lua_State* L)
	{
		// Args: texture_id, event_id
		// (texture_id is kept for API compatibility with builtin-japi but not strictly needed)
		/*int tex_id =*/ (void)lua_tointeger(L, 1);
		int ev_id = (int)lua_tointeger(L, 2);
		g_texture_events.erase(ev_id);
		return 0;
	}

	int open(lua_State* L)
	{
		lua_newtable(L);
		{
			lua_pushstring(L, "keyboard");
			init_keyboard(L);
			lua_rawset(L, -3);

			{
				luaL_Reg func[] = {
					{ "mouse", lmouse },
					{ "button", lbutton },
					{ "selection", lselection },
					// Screen-space mouse (builtin-japi: GetMouseVectorX/GetMouseVectorY)
					{ "mouse_screen", lmouse_screen },
					// Keyboard simulation (builtin-japi: SetKeyboard/ClickKeyboard)
					{ "set_keyboard", lset_keyboard },
					{ "click_keyboard", lclick_keyboard },
					// Console control (builtin-japi: ShowConsole)
					{ "show_console", lshow_console },
					// Texture event system (builtin-japi: TextureAddEvent)
					{ "texture_create", ltexture_create },
					{ "texture_destroy", ltexture_destroy },
					{ "texture_set_visible", ltexture_set_visible },
					{ "texture_set_trigger", ltexture_set_trigger },
					{ "texture_set_rect", ltexture_set_rect },
					{ "texture_add_event", ltexture_add_event },
					{ "texture_remove_event", ltexture_remove_event },
					{ NULL, NULL },
				};
				luaL_setfuncs(L, func, 0);
			}

			if (get_war3_searcher().get_version() >= version_124e)
			{
				luaL_Reg func[] = {
					{ "order_immediate", order::limmediate },
					{ "order_point", order::lpoint },
					{ "order_target", order::ltarget },
					{ "order_enable_debug", order::lenable_debug },
					{ NULL, NULL },
				};
				luaL_setfuncs(L, func, 0);
			}

			lua_newtable(L);
			{
				lua_pushstring(L, "__index");
				lua_pushcclosure(L, lget, 0);
				lua_rawset(L, -3);

				lua_pushstring(L, "__newindex");
				lua_pushcclosure(L, lset, 0);
				lua_rawset(L, -3);
			}
			lua_setmetatable(L, -2);

		}
		return 1;
	}
}
