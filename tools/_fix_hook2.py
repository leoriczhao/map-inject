"""Fix AbilityId_Hook - indentation + ENTER format string."""
import sys

path = r'C:\Users\leoric\Desktop\code\map-inject\japi\src\native_registry.cpp'

with open(path, 'rb') as f:
    content = f.read()

BS = ord('\\')
N = ord('n')
BN = bytes([BS, N])

# Fix the ENTER snprintf (line 180) - replace actual newline with backslash-n
old_enter = b'snprintf(buf, sizeof(buf), "[japi-ability] ENTER s=%s\n", s?s:"nil");'
new_enter = b'snprintf(buf, sizeof(buf), "[japi-ability] ENTER s=%s' + BN + b'", s?s:"nil");'

if old_enter in content:
    content = content.replace(old_enter, new_enter)
    print("Fixed ENTER snprintf")
else:
    print("ENTER snprintf already fixed or not found")
    # Check what's there
    idx = content.find(b'[japi-ability] ENTER')
    if idx >= 0:
        chunk = content[idx:idx+80]
        print(f"  Found: {repr(chunk)}")

# Fix indentation: the exec-lua block has 4 extra spaces
# Find the block
needle = b'if (s && strncmp(s, "exec-lua:", 9) == 0) {'
idx = content.find(needle)
if idx < 0:
    print("exec-lua block not found")
    sys.exit(1)

# Find end of block
passthrough = content.find(b'if (g_real_AbilityId)', idx)
last_return = content.rfind(b'return 0;', idx, passthrough)
after_return = content[last_return:passthrough]
brace_pos = after_return.find(b'}')
block_end = last_return + brace_pos + 1

old_block = content[idx:block_end]

# Rebuild block with correct indentation
new_block = (
    b'if (s && strncmp(s, "exec-lua:", 9) == 0) {\r\n'
    b'            int n = atoi(s + 9);\r\n'
    b'            lua_State* L = get_lua_state();\r\n'
    b'            char buf2[256];\r\n'
    b'            snprintf(buf2, sizeof(buf2), "[japi-ability] exec-lua:%d L=%p'
    + BN +
    b'", n, (void*)L);\r\n'
    b'            OutputDebugStringA(buf2); FILE* f2 = fopen("japi-debug.log", "a"); if(f2){fputs(buf2,f2);fclose(f2);}\r\n'
    b'            if (L && n >= 0 && n < 32) {\r\n'
    b'                japi::runtime::callback_read(L, n);\r\n'
    b'                int is_fn = lua_isfunction(L, -1);\r\n'
    b'                snprintf(buf2, sizeof(buf2), "[japi-ability] callback_read(%d) is_fn=%d'
    + BN +
    b'", n, is_fn);\r\n'
    b'                OutputDebugStringA(buf2); f2 = fopen("japi-debug.log", "a"); if(f2){fputs(buf2,f2);fclose(f2);}\r\n'
    b'                if (is_fn) {\r\n'
    b'                    int rc = lua_pcall(L, 0, 0, 0);\r\n'
    b'                    if (rc != LUA_OK) {\r\n'
    b'                        snprintf(buf2, sizeof(buf2), "[japi-ability] pcall error: %s'
    + BN +
    b'", lua_tostring(L, -1));\r\n'
    b'                        OutputDebugStringA(buf2); f2 = fopen("japi-debug.log", "a"); if(f2){fputs(buf2,f2);fclose(f2);}\r\n'
    b'                        lua_pop(L, 1);\r\n'
    b'                    }\r\n'
    b'                } else { lua_pop(L, 1); }\r\n'
    b'            }\r\n'
    b'            return 0;\r\n'
    b'        }'
)

content = content[:idx] + new_block + content[block_end:]

with open(path, 'wb') as f:
    f.write(content)

print("OK - indentation and format strings fixed")
