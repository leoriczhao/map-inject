#include <warcraft3/jass/hook.h>
#include <warcraft3/war3_searcher.h>
#include <warcraft3/hashtable.h>
#include <warcraft3/jass.h>
#include <warcraft3/jass/func_value.h>
#include <warcraft3/jass/nf_register.h>
#include <warcraft3/version.h>
#include <base/hook/replace_pointer.h>
#include <base/hook/iat.h>
#include <base/util/do_once.h>
#include <base/util/foreach.h>
#include <base/hook/fp_call.h>
#include <map>
#include <string>
#include <algorithm>

namespace warcraft3::jass {

	// list_hook: hook a JASS native by walking the native function linked list.
	// This is the same mechanism as the callback's CreateJassNativeHook.
	// The linked list is at: [[pJassEnv + 0x14] + 0x20].
	// Each native_func_node has: [+0]=vfn, [+4]=hash, [+8]=next/prev/lft/rht/..., [+20]=key(name ptr), [+24]=func_address
	// Does NOT use the hash table (which may not be available during callback context).
	bool list_hook(const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		uintptr_t env = get_war3_searcher().get_instance(5);
		if (!env) return false;

		// Get the native function linked list head: [[env+0x14]+0x20]
		uintptr_t table_ptr = *(uintptr_t*)(env + 0x14);
		if (!table_ptr) return false;
		uintptr_t first_node = *(uintptr_t*)(table_ptr + 0x20);
		if (!first_node) return false;

		// Walk the linked list, comparing node->key (name) with proc_name
		// native_func_node layout: vfn(4), hash(4), [linked list pointers](16), key(4), func_address_(4)
		// key is at offset 20, func_address_ is at offset 24
		uintptr_t current = first_node;
		for (int i = 0; i < 10000; i++) {
			// Check if this node is valid (func_address > 0x3000)
			uint32_t func_addr = *(uint32_t*)(current + 24);
			if (func_addr < 0x3000) break;

			// Compare key (name pointer) with our target name
			uint32_t key_ptr = *(uint32_t*)(current + 20);
			if (key_ptr > 0x10000) {
				const char* name = (const char*)key_ptr;
				if (strcmp(name, proc_name) == 0) {
					*old_proc_ptr = (uintptr_t)func_addr;
					*(uint32_t*)(current + 24) = (uint32_t)new_proc;
					return true;
				}
			}

			// Follow the linked list: prev_ is at offset 8 (based on node structure)
			// The callback uses: ReadRealMemory(NextAddress) for next, which is the first field
			// But native_func_node inherits from virtual_func_table + node, so:
			//   [+0] = vfn_ (virtual func table)
			//   [+4] = hash_
			//   [+8] = next_ (node_1*)
			//   [+12] = prev_ (node*)  <-- callback uses this as "next" in linked list traversal
			// Actually, the callback reads NextAddress[0] as the next pointer.
			// For native_func_node, the first non-vtable field after inheritance...
			// Let me just try offset 0 as the next pointer (like the callback does)
			uintptr_t next = *(uintptr_t*)(current);
			if (next == first_node || next == 0 || next < 0x10000) break;
			current = next;
		}
		return false;
	}

	bool table_hook     (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		if (!get_war3_searcher().get_instance(5))
			return false;

		if (!old_proc_ptr)
			return false;

		hashtable::native_func_node* node_ptr = get_native_function_hashtable()->find(proc_name);

		if (!node_ptr)
			return false;

		*old_proc_ptr = (uintptr_t)node_ptr->func_address_;
		node_ptr->func_address_  = (uint32_t)new_proc;

		return true;
	}

	bool table_unhook   (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t /*new_proc*/)
	{
		if (!old_proc_ptr)
			return false;

		hashtable::native_func_node* node_ptr = get_native_function_hashtable()->find(proc_name);

		if (!node_ptr)
			return false;

		node_ptr->func_address_ = (uint32_t)(*old_proc_ptr);

		return true;
	}

	bool register_hook  (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		if (!old_proc_ptr || !new_proc) 
			return false;

		uintptr_t hook_address = get_war3_searcher().search_string(proc_name);
		if (!hook_address)
			return false;

		uintptr_t old_proc = base::hook::replace_pointer(hook_address + 0x05, new_proc);
		if (!old_proc)
			return false;

		*old_proc_ptr = old_proc;
		return true;
	}

	bool register_unhook(const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t /*new_proc*/)
	{
		if (!old_proc_ptr) 
			return false;

		uintptr_t hook_address = get_war3_searcher().search_string(proc_name);
		if (!hook_address)
			return false;

        base::hook::replace_pointer(hook_address + 0x05, *old_proc_ptr);
		return true;
	}

	namespace detail {
		struct add_info
		{
			uintptr_t   func;
			std::string param;

			add_info()
				: func(0)
				, param()
			{ }

			void set(uintptr_t f, const char* p)
			{
				func = f;
				param = p;
			}
		};

		struct hook_info
		{
			uintptr_t*  old_proc_ptr;
			uintptr_t   new_proc;
			bool        japi;

			hook_info()
				: old_proc_ptr(0)
				, new_proc(0)
				, japi(false)
			{ }

			void set(uintptr_t* o, uintptr_t n, bool j)
			{
				old_proc_ptr = o;
				new_proc = n;
				japi = j;
			}
		};

		std::map<std::string, add_info>  add_info_list;
		std::map<std::string, hook_info> hook_info_list;
		std::map<std::string, hook_info> once_hook_info_list;
		
		uintptr_t search_register_func()
		{
			uintptr_t ptr = get_war3_searcher().search_string("StringCase");
			ptr += 9;
			return convert_function(ptr);
		}

		void async_add(uintptr_t func, const char* name, const char* param)
		{
			add_info_list[name].set(func, param);
		}

		void async_hook(const char* name, uintptr_t* old_proc_ptr, uintptr_t new_proc, bool japi)
		{
			hook_info_list[name].set(old_proc_ptr, new_proc, japi);
		}
		
		void async_unhook(const char* name)
		{
			hook_info_list.erase(name);
		}

		void async_once_hook(const char* name, uintptr_t* old_proc_ptr, uintptr_t new_proc, bool japi)
		{
			once_hook_info_list[name].set(old_proc_ptr, new_proc, japi);
		}

		void async_once_unhook(const char* name)
		{
			once_hook_info_list.erase(name);
		}

		void async_initialize()
		{
			nf_register::initialize();
		}

	}

	bool japi_table_add(uintptr_t func, const char* name, const char* param)
	{
		static uintptr_t register_func = detail::search_register_func();
        base::fast_call<void>(register_func, func, name, param);
		japi_func_add(name, func, param);
		return true;
	}

	void nfunction_add()
	{
		japi_func_clean();
		foreach(auto const& it, detail::add_info_list)
		{
			japi_table_add(it.second.func, it.first.c_str(), it.second.param.c_str());
		}
	}

	void nfunction_hook()
	{
		foreach(auto const& it, detail::hook_info_list)
		{
			table_hook(it.first.c_str(), it.second.old_proc_ptr, it.second.new_proc);
			if (it.second.japi) {
				japi_func_add(it.first.c_str(), it.second.new_proc);
			}
		}
		foreach(auto const& it, detail::once_hook_info_list)
		{
			table_hook(it.first.c_str(), it.second.old_proc_ptr, it.second.new_proc);
			if (it.second.japi) {
				japi_func_add(it.first.c_str(), it.second.new_proc);
			}
		}
		detail::once_hook_info_list.clear();
	}

	bool async_hook           (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		detail::async_initialize();
		detail::async_hook(proc_name, old_proc_ptr, new_proc, false);
		return true;
	}

	bool async_once_hook       (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		detail::async_initialize();
		detail::async_once_hook(proc_name, old_proc_ptr, new_proc, false);
		return true;
	}

	bool async_once_unhook       (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		old_proc_ptr;
		new_proc;
		detail::async_initialize();
		detail::async_once_unhook(proc_name);
		return true;
	}

	bool async_unhook         (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		old_proc_ptr;
		new_proc;
		detail::async_initialize();
		detail::async_unhook(proc_name);
		return true;
	}

	bool japi_hook           (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		detail::async_initialize();
		detail::async_hook(proc_name, old_proc_ptr, new_proc, true);
		return true;
	}

	bool japi_unhook         (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		old_proc_ptr;
		new_proc;
		detail::async_initialize();
		detail::async_unhook(proc_name);
		return true;
	}

	bool japi_once_hook(const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc)
	{
		detail::async_initialize();
		detail::async_once_hook(proc_name, old_proc_ptr, new_proc, true);
		return true;
	}

	uint32_t hook           (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc, uint32_t flag)
	{
		uint32_t result = 0;

		if (flag & HOOK_MEMORY_TABLE)
		{
			if (table_hook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_MEMORY_TABLE;
			}
		}

		if (flag & HOOK_CODE_REGISTER)
		{
			if (register_hook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_MEMORY_TABLE;
			}
		}

		if (flag & HOOK_MEMORY_REGISTER)
		{
			if (async_hook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_MEMORY_TABLE;
			}
		}

		if (flag & HOOK_ONCE_MEMORY_REGISTER)
		{
			if (result == 0)
			{
				if (async_once_hook(proc_name, old_proc_ptr, new_proc))
				{
					result |= HOOK_ONCE_MEMORY_REGISTER;
				}
			}
			else
			{
				result |= HOOK_ONCE_MEMORY_REGISTER;
			}
		}

		return result;
	}

	uint32_t unhook         (const char* proc_name, uintptr_t* old_proc_ptr, uintptr_t new_proc, uint32_t flag)
	{
		uint32_t result = 0;

		if (flag & HOOK_MEMORY_TABLE)
		{
			if (table_unhook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_MEMORY_TABLE;
			}
		}

		if (flag & HOOK_CODE_REGISTER)
		{
			if (register_unhook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_MEMORY_TABLE;
			}
		}

		if (flag & HOOK_MEMORY_REGISTER)
		{
			if (async_unhook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_MEMORY_TABLE;
			}
		}
		
		if (flag & HOOK_ONCE_MEMORY_REGISTER)
		{
			if (async_once_unhook(proc_name, old_proc_ptr, new_proc))
			{
				result |= HOOK_ONCE_MEMORY_REGISTER;
			}
		}

		return result;
	}

	bool japi_add            (uintptr_t func, const char* name, const char* param)
	{
		detail::async_initialize();
		detail::async_add(func, name, param);
		return true;
	}
}
