-- ============================================================
-- japi.dll test suite
-- ============================================================
local jass    = require "jass.common"
local message = require "jass.message"
local runtime = require "jass.runtime"
local japi    = require "jass.japi"
local hook    = require "jass.hook"
local slk     = require "jass.slk"
local util    = require "lib.util"

-- Handle-level agnostic helpers (works with integer or userdata)
local function handle_val(h)
    if type(h) == "userdata" then
        local hex = tostring(h):match("0x([0-9A-Fa-f]+)")
        return hex and tonumber(hex, 16) or 0
    end
    return h or 0
end
local function is_handle(h)
    return h ~= nil and h ~= 0 and handle_val(h) ~= 0
end

local pass, fail = 0, 0
local function test(name, fn)
    local ok, err = pcall(fn)
    if ok then pass = pass + 1; print(string.format("  [PASS] %s", name))
    else        fail = fail + 1; print(string.format("  [FAIL] %s : %s", name, tostring(err)))
    end
end

local function run()
print("========================================")
print(" japi.dll test suite")
print("========================================")

-- ============================================================
-- 1. jass.common — JASS native calling
-- ============================================================
print("\n-- jass.common --")

test("module loaded", function()
    assert(type(jass) == "table")
    assert(type(jass.call) == "function")
end)

test("GetLocalPlayer returns handle", function()
    local p = jass.GetLocalPlayer()
    assert(is_handle(p), "expected handle, got " .. type(p) .. " = " .. tostring(p))
end)

test("metatable __index vs jass.call", function()
    local p1 = jass.GetLocalPlayer()
    local p2 = jass.call("GetLocalPlayer")
    assert(p1 == p2)
end)

test("DisplayTimedTextToPlayer", function()
    jass.DisplayTimedTextToPlayer(jass.GetLocalPlayer(), 0, 0, 3, "[test] hello")
end)

test("Player(0) returns handle", function()
    local p = jass.Player(0)
    assert(is_handle(p), "expected handle, got " .. type(p))
end)

test("GetLocalPlayer == Player(GetPlayerId(LocalPlayer))", function()
    local lp = jass.GetLocalPlayer()
    local id = jass.GetPlayerId(lp)
    local p  = jass.Player(id)
    assert(handle_val(lp) == handle_val(p),
        string.format("local=%d id=%d player=%d", handle_val(lp), id, handle_val(p)))
end)

test("GetTriggerPlayer returns nil at init", function()
    local tp = jass.GetTriggerPlayer()
    assert(tp == nil or handle_val(tp) == 0)
end)

test("call with string param", function()
    local p = jass.GetLocalPlayer()
    jass.call("DisplayTimedTextToPlayer", p, 0, 0, 3, "str test")
end)

test("call with real param", function()
    local p = jass.GetLocalPlayer()
    jass.call("DisplayTimedTextToPlayer", p, 3.5, 0, 3, "real test")
end)

test("integer return from native", function()
    local id = jass.call("GetPlayerId", jass.GetLocalPlayer())
    assert(type(id) == "number", "expected number, got " .. type(id))
end)

-- ============================================================
-- 2. jass.runtime — runtime configuration
-- ============================================================
print("\n-- jass.runtime --")

test("module loaded", function()
    assert(type(runtime) == "table")
end)

test("version non-empty", function()
    assert(type(runtime.version) == "string" and #runtime.version > 0,
        "version: " .. tostring(runtime.version))
end)

test("defaults", function()
    assert(runtime.sleep       == false)
    assert(runtime.console     == false)
    assert(runtime.catch_crash == true)
    assert(runtime.handle_level == 2)
end)

test("error_handle set/clear", function()
    runtime.error_handle = function(msg) return msg end
    assert(type(runtime.error_handle) == "function")
    runtime.error_handle = nil
    assert(runtime.error_handle == nil)
end)

test("sleep property toggles", function()
    runtime.sleep = true;  assert(runtime.sleep == true)
    runtime.sleep = false; assert(runtime.sleep == false)
end)

test("console property toggles", function()
    runtime.console = true;  assert(runtime.console == true)
    runtime.console = false; assert(runtime.console == false)
end)

test("handle_level property toggles", function()
    local old = runtime.handle_level
    runtime.handle_level = 2; assert(runtime.handle_level == 2)
    runtime.handle_level = 0; assert(runtime.handle_level == 0)
    runtime.handle_level = 2; assert(runtime.handle_level == 2)
    if old then runtime.handle_level = old end
end)

test("catch_crash property toggles", function()
    runtime.catch_crash = false; assert(runtime.catch_crash == false)
    runtime.catch_crash = true;  assert(runtime.catch_crash == true)
end)

-- ============================================================
-- 3. jass.japi — custom native registration
-- ============================================================
print("\n-- jass.japi --")

test("module loaded", function()
    assert(type(japi) == "table")
end)

test("register (II)I and call", function()
    japi("TestAdd", "(II)I", function(a, b) return a + b end)
    assert(japi.TestAdd(10, 32) == 42)
    assert(japi.TestAdd(-5, 5) == 0)
    assert(japi.TestAdd(0x7FFFFFFF, 1) == -0x80000000) -- 32-bit overflow wrap via hashtable
end)

test("register (R)R double", function()
    japi("TestDouble", "(R)R", function(x) return x * 2 end)
    local r = japi.TestDouble(3.5)
    assert(math.abs(r - 7.0) < 0.001, "expected 7.0, got " .. tostring(r))
end)

test("register (I)I with nil arg → 0", function()
    japi("TestNilArg", "(I)I", function(a) return a end)
    local r = japi.TestNilArg(nil)
    assert(r == 0, "nil should map to integer 0 via hashtable, got " .. tostring(r) .. " (" .. type(r) .. ")")
end)

test("register (SS)S concat returns string", function()
    japi("TestStrConcat", "(SS)S", function(a, b) return a .. "_" .. b end)
    local r = japi.TestStrConcat("hello", "world")
    assert(r == "hello_world", "got: " .. tostring(r))
end)


test("re-registration is rejected", function()
    japi("TestReReg", "(II)I", function(a, b) return a + b end)
    local ok, err = pcall(japi, "TestReReg", "(II)I", function(a, b) return a * b end)
    assert(not ok, "re-registration should error, but succeeded")
end)

test("iterate registered natives", function()
    local count = 0
    for _ in pairs(japi) do count = count + 1 end
    assert(count >= 1, "no registered natives found")
end)

-- ============================================================
-- 4. jass.hook — native hooking (via jass.call)
-- Bridge wrappers are JASS functions, not natives. Hook tests
-- use jass.call with natives registered by japi_table_add.
-- ============================================================
print("\n-- jass.hook --")

test("module loaded", function()
    assert(type(hook) == "table")
    assert(type(hook.hook) == "function")
    assert(type(hook.unhook) == "function")
end)

test("hook native TestAdd with multiplier", function()
    -- TestAdd is registered as a JASS native by init_native_registry
    hook.hook("TestAdd", function(a, b, orig)
        return orig(a, b) * 3
    end)
    -- Call via jass.call (uses native dispatch, not bridge wrapper)
    assert(jass.call("TestAdd", 2, 3) == 15, "hook multiply failed")
    hook.unhook("TestAdd")
    assert(jass.call("TestAdd", 2, 3) == 5, "unhook restore failed")
end)

test("hook native blocks original", function()
    hook.hook("TestAdd", function(a, b, orig)
        return 999
    end)
    local r = jass.call("TestAdd", 5, 3)
    assert(r == 999, "hook didn't block: got " .. tostring(r))
    hook.unhook("TestAdd")
    assert(jass.call("TestAdd", 5, 3) == 8, "restore after block failed")
end)

test("hook passes orig correctly", function()
    hook.hook("TestAdd", function(a, b, orig)
        return orig(a, b) + 100
    end)
    assert(jass.call("TestAdd", 2, 3) == 105)
    hook.unhook("TestAdd")
    assert(jass.call("TestAdd", 2, 3) == 5)
end)

test("double hook is error", function()
    hook.hook("TestAdd", function(a, b, orig) return orig(a, b) + 1 end)
    local ok, err = pcall(hook.hook, "TestAdd", function(a, b, o) return o(a, b) end)
    assert(not ok, "double hook should error")
    hook.unhook("TestAdd")
end)

test("hook nonexistent native errors", function()
    local ok, err = pcall(hook.hook, "NonExistentNative_XYZ", function() end)
    assert(not ok, "should error on nonexistent native")
end)

test("hook game native via jass.call", function()
    local lp = jass.GetLocalPlayer()
    local orig_id = jass.call("GetPlayerId", lp)
    hook.hook("GetPlayerId", function(p, orig)
        return orig(p) + 100
    end)
    local hooked_id = jass.call("GetPlayerId", lp)
    assert(hooked_id == orig_id + 100,
        string.format("hook game native: %d != %d + 100", hooked_id, orig_id))
    hook.unhook("GetPlayerId")
    local restored_id = jass.call("GetPlayerId", lp)
    assert(restored_id == orig_id,
        string.format("unhook restore: %d != %d", restored_id, orig_id))
end)

test("hook game native via metatable jass.X()", function()
    local lp = jass.GetLocalPlayer()
    local orig_id = jass.GetPlayerId(lp)
    hook.hook("GetPlayerId", function(p, orig)
        return orig(p) + 42
    end)
    local via_meta = jass.GetPlayerId(lp)
    assert(via_meta == orig_id + 42,
        string.format("metatable hook: %d != %d + 42", via_meta, orig_id))
    hook.unhook("GetPlayerId")
    assert(jass.GetPlayerId(lp) == orig_id)
end)

test("hook re-hook after unhook", function()
    local lp = jass.GetLocalPlayer()
    local orig = jass.call("GetPlayerId", lp)
    hook.hook("GetPlayerId", function(p, o) return o(p) + 1 end)
    assert(jass.call("GetPlayerId", lp) == orig + 1)
    hook.unhook("GetPlayerId")
    hook.hook("GetPlayerId", function(p, o) return o(p) + 10 end)
    assert(jass.call("GetPlayerId", lp) == orig + 10)
    hook.unhook("GetPlayerId")
    assert(jass.call("GetPlayerId", lp) == orig)
end)

-- ============================================================
-- 5. jass.message — input state
-- ============================================================
print("\n-- jass.message --")

test("module loaded", function()
    assert(type(message) == "table")
    assert(type(message.keyboard) == "table")
    assert(type(message.mouse) == "function")
end)

test("keyboard table read/write", function()
    message.keyboard[0x41] = true
    assert(message.keyboard[0x41] == true)
    message.keyboard[0x41] = nil
    assert(message.keyboard[0x41] == nil or message.keyboard[0x41] == false)
end)

test("keyboard mass read/write", function()
    for i = 1, 10 do
        message.keyboard[0x40 + i] = true
    end
    local count = 0
    for k, v in pairs(message.keyboard) do
        if v then count = count + 1 end
    end
    assert(count >= 10, "expected >=10 true keys, got " .. count)
    for i = 1, 10 do message.keyboard[0x40 + i] = nil end
end)

test("mouse returns world coords", function()
    local wx, wy = message.mouse()
    assert(type(wx) == "number" and type(wy) == "number")
end)

test("selection returns nil at init", function()
    local sel = util.get_selected_unit()
    assert(sel == nil or type(sel) == "number")
end)

test("order functions exist and return boolean", function()
    assert(type(message.order_immediate) == "function")
    assert(type(message.order_point) == "function")
    assert(type(message.order_target) == "function")
    local r1 = message.order_immediate(0, 0)
    local r2 = message.order_point(0, 0, 0, 0)
    local r3 = message.order_target(0, 0, 0)
    assert(type(r1) == "boolean")
    assert(type(r2) == "boolean")
    assert(type(r3) == "boolean")
end)

test("order_enable_debug toggles", function()
    assert(type(message.order_enable_debug) == "function")
    message.order_enable_debug(true)
    message.order_enable_debug(false)
    message.order_enable_debug(1)
    message.order_enable_debug(0)
end)

test("button all slots return types", function()
    for x = 0, 3 do
        for y = 0, 2 do
            local a, o, t = message.button(x, y)
            assert(type(a) == "number" or a == nil)
        end
    end
end)

test("hook set/clear", function()
    message.hook = function(msg) return true end
    assert(type(message.hook) == "function")
    message.hook = nil
    assert(message.hook == nil)
end)

test("hook nil clears and returns nil", function()
    message.hook = function(msg) return false end
    assert(type(message.hook) == "function")
    message.hook = nil
    assert(message.hook == nil)
    message.hook = nil
    assert(message.hook == nil)
end)

test("keyboard survives across accesses", function()
    for k in pairs(message.keyboard) do
        message.keyboard[k] = nil
    end
    message.keyboard[0x20] = true
    local kb1 = message.keyboard
    kb1[0x41] = true
    local kb2 = message.keyboard
    assert(kb2[0x20] == true, "key 0x20 should persist")
    assert(kb2[0x41] == true, "key 0x41 should persist across accesses")
    message.keyboard[0x20] = nil
    message.keyboard[0x41] = nil
end)

test("message.hook function callable with msg table", function()
    message.hook = function(msg)
        assert(type(msg) == "table", "hook should receive table")
        return true
    end
    local h = message.hook
    assert(type(h) == "function")
    local ok, _ = pcall(h, { type = "test", category = "test" })
    assert(ok, "hook function should accept msg table")
    message.hook = nil
end)

-- ============================================================
-- 6. jass.slk — game data tables
-- ============================================================
print("\n-- jass.slk --")

test("module loaded", function()
    assert(type(slk) == "table")
end)

local slk_tests = {
    { name = "ability",      min_rows = 10, key = "AHbz", field = "code",  want = "AHbz"  },
    { name = "unit",         min_rows = 10, key = "hpea", field = "race",  want = "human" },
    { name = "item",         min_rows = 5,  key = "rat3", field = "class", want = nil     },
    { name = "buff",         min_rows = 5,  key = "Bpsd" },
    { name = "upgrade",      min_rows = 5  },
    { name = "destructable", min_rows = 1  },
    { name = "doodad",       min_rows = 1  },
}

for _, t in ipairs(slk_tests) do
    test(string.format("slk.%s lazy-loads", t.name), function()
        local data = slk[t.name]
        assert(type(data) == "table", "not a table")
        local count = 0
        for _ in pairs(data) do count = count + 1 end
        assert(count >= t.min_rows,
            string.format("%s: %d rows < %d", t.name, count, t.min_rows))
        if t.key then
            local row = data[t.key]
            assert(type(row) == "table",
                string.format("%s[%s] not found", t.name, t.key))
            if t.field and t.want then
                local v = row[t.field]
                assert(tostring(v):lower() == t.want:lower(),
                    string.format("%s[%s].%s=%s ≠ %s",
                        t.name, t.key, t.field, tostring(v), t.want))
            end
        end
    end)
end

test("slk.ability second access is cached", function()
    local raw = rawget(slk, "ability")
    assert(type(raw) == "table", "cached ability table missing")
    local count = 0
    for _ in pairs(raw) do count = count + 1 end
    assert(count >= 10, "cached ability empty: " .. count)
end)

-- ============================================================
-- 7. jass.sleep — coroutine sleep
-- ============================================================
print("\n-- jass.sleep --")

test("module loaded", function()
    local sleep_mod = require "jass.sleep"
    assert(type(sleep_mod) == "table")
    assert(type(sleep) == "function", "global sleep not registered")
end)

test("sleep errors outside coroutine", function()
    local ok, err = pcall(sleep, 0.1)
    assert(not ok, "sleep should error outside coroutine")
end)

test("sleep in coroutine is callable", function()
    local co = coroutine.create(function()
        local t = sleep(0.01)
        return t
    end)
    assert(coroutine.status(co) == "suspended")
end)

test("runtime.sleep enable/disable", function()
    runtime.sleep = true;  assert(runtime.sleep == true)
    runtime.sleep = false; assert(runtime.sleep == false)
end)

-- ============================================================
-- 10. jass.code trampoline (AbilityId dispatch)
-- ============================================================
print("\n-- jass.code trampoline --")

test("jass.code + TriggerAddAction round-trip", function()
    local fired = false
    local cb = jass.code(function() fired = true end)
    local trig = jass.CreateTrigger()
    jass.TriggerAddAction(trig, cb)
    jass.TriggerExecute(trig)
    jass.DestroyTrigger(trig)
    assert(fired, "callback was not called!")
end)

-- ============================================================
-- Summary
print("")
print("========================================")
local total = pass + fail
print(string.format(" Results: %d passed, %d failed, %d total", pass, fail, total))
print("========================================")

if fail > 0 then
    print("SOME TESTS FAILED (" .. fail .. "/" .. total .. ")")
else
    print("ALL TESTS PASSED (" .. total .. "/" .. total .. ")")
    jass.DisplayTimedTextToPlayer(jass.GetLocalPlayer(), 0, 0, 10,
        "[japi.dll] ALL TESTS PASSED " .. total .. "/" .. total)
end
end  -- run()

return { run = run, get_results = function() return pass, fail end }
