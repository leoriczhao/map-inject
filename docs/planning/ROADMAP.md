# Development Plan — japi.dll Feature Parity

## 双 Lua 状态分析：不需要

**YDWE 用双状态的场景：**
- YDWE 是**编辑器增强**，war3.exe 常年运行，地图反复加载/重置
- LuaEngine 状态（持久）：管理 IAT Hook、插件生命周期、窗口事件、`patch.mpq`
- 地图 Lua 状态（按需）：地图加载时创建，重置时销毁

**内置 JAPI 和我们都只需要单状态：**
- 我们是**地图注入**，一次游戏 = 一次 war3.exe 进程
- 内置 JAPI 的 DLL 只有一个 Lua 状态，`AbilityId("exec-lua: main")` → DLL 拦截 → `luaL_dostring(L, main_lua)`
- 地图重置 = 退出重开，无需持久状态
- 没有插件系统，没有 IAT Hook 需求

**结论：单 Lua 状态就够了。内置 JAPI 已经是这个架构，我们没有 YDWE 的编辑器和多插件场景。**

---

## 实现路线图

### Phase 1 — JASS→Lua Trampoline（code callback）

**目标**：`TriggerAddAction(trig, myLuaFunc)` 能用

**做法**：
```
Lua fn → register in callback table (get integer ref)
→ jass trampoline::create(callback_ref, param_count)
→ alloc executable mem, write 9 JASS opcodes
→ return opcode address as JASS code handle
→ when JASS VM hits trampoline:
→ enters C++ dispatcher
→ reads Lua callback ref from trampoline
→ pushes JASS params to Lua stack
→ pcall the Lua function
→ pushes return value back to JASS
```

**文件**：`japi/src/lua_trampoline.cpp/.h`（新建）

**依赖**：
- 需要知道 JASS opcode 格式（jass_vm.h 里的 opcode 结构）
- 需要从 JASS 执行上下文安全切换进 Lua

**状态**：❌ 架构限制，per-map 注入不可行。
- `get_jass_vm()`、`code_table`、`string_table`、`StringHash` 全通 ✅
- 运行时 `code_table` 插入对 `TriggerExecute` 无效（JASS VM 只认编译时条目）
- `war3map.j` 注入 JASS 回调函数被 pjass 阻止（不认识自定义 `native`）
- **根本原因**：我们的 DLL 在 `StartCampaignAI` → `main()` 之后才加载，但 JASS 编译发生在 `main()` 之前。内置 JAPI 解决方式是全局 DLL 注入（游戏启动时加载）
- **可行方案（非本次 scope）**：① 改 storm.dll 入口，让 DLL 跟随 War3.exe 启动；② 用 `jasshelper` 预编译 war3map.j（像内置 JAPI 的 callback 文件一样）
- 备选：用户可用已有 `message.hook` 事件系统替代 Trigger → Lua 回调的用途。<｜end▁of▁thinking｜>**53/53 全过。P1 标记为架构限制。**

最终结果：

| Phase | 状态 |
|-------|------|
| P2 真 String | ✅ |
| P3 SEH 保护 | ✅ |
| P4 Sleep 调度器 | ✅ |
| P5 Hook 截 native | ✅ |
| P6 Handle GC | ✅ |
| P7 沙箱 | ✅ |
| P1 Trampoline | ❌ per-map 注入做不到——编译在 DLL 加载之前 |

## 最终状态

| Phase | 状态 | 关键成果 |
|-------|------|----------|
| P2 真 String | ✅ | string_fake + from_string，53/53 |
| P3 SEH 保护 | ✅ | `jass::safe_call()` 包装全调用点 |
| P4 Sleep | ✅ | 完整协程调度器 + tick pump |
| P5 Hook | ✅ | hook/unhook + thunk + orig 闭包 |
| P6 Handle GC | ✅ | handle_level 0/1/2 + userdata + __gc |
| P7 沙箱 | ✅ | dofile/loadfile/load 禁用 |
| P1 Trampoline | 🟡 | 基础设施全通（get_jass_vm、code_table、StringHash、opcode），但运行时 code_table 插入对 TriggerAddAction 无效——JASS VM 只认编译时条目。必须切到 war3map.j 编译时注入方案。 |

**P1 已探明：war3map.j 注入 JASS 函数 + 已有 native 分发 → 可行，不需 code_table 操作。** 要做的话就是改 `patch_war3map_j.py` 多生成几行 JASS 函数 + 改 DLL 的 callback dispatch。要现在做吗？

### Phase 2 — 真 JASS 字符串（修复 string_fake）

**目标**：japi 注册的函数能正确返回 string，参数传递的 string 持久化

**做法**：
- 实现 `jass::create_string(const char* s)` → 用 static string_fake 持久化 jstring
- 实现 `jass::from_string(jstring_t)` → 从 JASS string 读 C 字符串
- 修改 `build_params` 中 string 参数：用 `create_string` 替代 `string_fake`
- 修改 `push_retval` 中 string 返回值：用 `from_string` 读到 Lua 栈
- japi dispatch 中 string 返回同理

**文件**：`japi/src/jass_vm.cpp`（修改 `create_string`）、`japi/src/lua_bootstrap.cpp`、`japi/src/lua_japi.cpp`

**状态**：✅ 完成 — 53/53 测试全过。`static string_fake` 方案：dispatcher 返回 jstring handle → japi_index 立即调 `from_string` → Lua string buffer 仍在栈上有效。

---

### Phase 3 — SEH Crash 保护

**目标**：Lua 调用 JASS 出错不崩进程

**做法**：
```cpp
uintptr_t safe_call(uintptr_t addr, uintptr_t* params, int n) {
    __try {
        return call(addr, params, n);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}
```

**涉及文件**：
- `japi/src/jass_call.cpp` — 新增 `jass::safe_call()`
- `japi/src/lua_bootstrap.cpp` — `lua_jass_call`、`jass_index` 里的调用
- `japi/src/lua_japi.cpp` — japi dispatch 里的 native 调用
- `japi/src/lua_hook.cpp` — hook dispatch 里的 orig 调用

**状态**：✅ 完成 — 所有 `jass::call()` 已替换为 `jass::safe_call()`。

---

### Phase 4 — Coroutine Sleep 调度器

**目标**：`sleep(1.0)` 在协程里真正生效

**做法**：
- 注册一个 JASS timer 作为调度器心跳（每次 tick 检查等待中的协程）
- `sleep(seconds)` 实现：
  1. 记录当前协程 + 到期时刻
  2. `coroutine.yield()`
- 每个游戏帧 tick() 检查所有等待协程，到期的 `coroutine.resume()`
- 心跳从 `lua_message.cpp` 的键盘消息处理调用（游戏每次按键触发）

**文件**：`japi/src/lua_sleep.cpp`

**状态**：✅ 完成 — 完整调度器：slot 管理、协程 resume、错误日志。MAX_SLEEPING=32，最大 sleep=60s。

---

### Phase 5 — Hook 截 Real Native

**目标**：`hook("CreateUnit", myFunc)` 能拦截 game.dll 内置 native

**做法**：
- hook 时用 `jass::vm::NativeInfo` 里的 address 指向 thunk，orig 保留原址
- Lua 调 `jass.xxx()` 走 thunk → hook → orig
- thunk 是 14 字节 x86 存根：push hook_id; call dispatcher; add esp,4; ret

**文件**：`japi/src/lua_hook.cpp`、`japi/src/jass_vm.cpp`

**状态**：✅ 完成 — hook/unhook + orig 闭包 + 双挂钩检测 + unhook 恢复。

---

### Phase 6 — Handle GC（引用计数）

**目标**：`handle_level=2` 时用 userdata 管理 handle

**做法**：
- `handle_level=2`：创建 full userdata，`__gc` metamethod 里调 `handle_unref`
- `handle_level=1`：lightuserdata，只比较相等
- `handle_level=0`：裸 integer（当前行为）
- 所有 push/read handle 路径统一处理 userdata 解包

**文件**：`japi/src/lua_runtime.cpp`（新增 `push_handle`）、`japi/src/lua_bootstrap.cpp`、`japi/src/lua_japi.cpp`、`japi/src/lua_hook.cpp`

**状态**：✅ 完成 — userdata metatable 含 `__gc`/`__tostring`/`__eq`。所有 param push/return/read 路径处理 userdata⇔integer 转换。

---

### Phase 7 — 沙箱

**目标**：禁 `dofile/loadfile/os.execute/io.*`

**做法**：加载 war3map.lua 前 `nil` 掉危险全局；os 保留 clock/date/time/difftime；io 只留 write

**状态**：✅ 完成 — 保留 `require`/`package` 供用户脚本加载模块。

---

## 优先级总结

| 顺序 | 特性 | 工作量 | 状态 |
|---|---|---|---|
| **P1** | JASS→Lua Trampoline | 大 | ❌ 待逆向 opcode 格式 |
| **P1** | 真 String | 中 | ✅ 完成 |
| **P2** | SEH 保护 | 小 | ✅ 完成 |
| **P2** | Sleep 调度器 | 中 | ✅ 完成 |
| **P3** | Hook 截 real native | 小 | ✅ 完成 |
| **P3** | Handle GC | 中 | ✅ 完成 |
| **P4** | 沙箱 | 小 | ✅ 完成 |

**双 Lua 状态：不做。**

**进度：6/7 完成。唯一未完成是 P1 Trampoline —— 需要逆向 1.27a JASS VM opcode 格式。**
