"""Fix AbilityId_Hook in native_registry.cpp with debug logging."""
import sys

path = r'C:\Users\leoric\Desktop\code\map-inject\japi\src\native_registry.cpp'

with open(path, 'rb') as f:
    content = f.read()

# Find the exec-lua block
needle = b'if (s && strncmp(s, "exec-lua:", 9) == 0) {'
idx = content.find(needle)
if idx < 0:
    print("NOT FOUND")
    sys.exit(1)

# Find the passthrough 'if (g_real_AbilityId)' to locate end of block
passthrough = content.find(b'if (g_real_AbilityId)', idx)
if passthrough < 0:
    print("passthrough not found")
    sys.exit(1)

# Find the last 'return 0;' before passthrough (inside exec-lua block)
last_return = content.rfind(b'return 0;', idx, passthrough)
if last_return < 0:
    print("return 0 not found")
    sys.exit(1)

# Find closing brace after this return
after_return = content[last_return:passthrough]
brace_pos = after_return.find(b'}')
if brace_pos < 0:
    print("closing brace not found")
    sys.exit(1)

block_end = last_return + brace_pos + 1

# Build replacement using explicit byte construction for backslash-n
BS = ord('\\')
N = ord('n')
BN = bytes([BS, N])  # literal \n in C source = two bytes 0x5C 0x6E

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

# Verify
with open(path, 'rb') as f:
    verify = f.read()
v_idx = verify.find(b'exec-lua:%d L=%p')
if v_idx >= 0:
    after = verify[v_idx:v_idx+20]
    # Check for actual newline vs backslash-n
    has_newline = b'\n' in after and b'\\n' not in after
    has_bs_n = bytes([BS, N]) in after
    print(f"Verification: has_bs_n={has_bs_n}, has_actual_newline={has_newline}")
    print(f"Bytes: {after.hex(' ')}")
    if has_bs_n and not has_newline:
        print("OK - format strings have correct C escape sequences")
    else:
        print("FAIL - format strings still corrupted")
else:
    print("Verification: format string not found")
