# YDWE LuaEngine 注入与双虚拟机架构分析

## 一、总体概述

YDWE（YD Warrior Editor）是一个增强的 Warcraft III 世界编辑器。它在不修改 `Game.dll` 二进制文件的前提下，通过 **代码洞注入（Code Cave Injection）** 将一个完整的 Lua 5.4 运行时注入到 `war3.exe` 进程中，并与 Warcraft III 原生的 JASS 虚拟机并行运行，形成"双虚拟机"架构。

整个系统分为四个阶段：
1. **进程启动与注入** — `war3.exe` 以挂起状态创建，`LuaEngine.dll` 通过 shellcode 注入
2. **Lua 引导系统** — `war3/main.lua` 通过 IAT Hook 拦截 `Game.dll` 加载，并加载所有插件
3. **JASS-Lua 桥接** — `yd_lua_engine.dll` 建立双向调用通道
4. **双虚拟机并行运行** — JASS 和 Lua 同时执行，通过 trampoline 和 coroutine 机制互操作

---

## 二、第一阶段：进程启动与代码洞注入

### 2.1 启动流程

```
YDWE.exe
  └─ 加载 YDWEStartup.dll，调用 YDWEStartup() 导出函数
       └─ launch_warcraft3()
            ├─ 设置环境变量 ydwe-process-name = "war3"
            ├─ bee::subprocess::spawn.suspended() → 创建挂起的 war3.exe 进程
            ├─ 可选：hook::replacedll("Storm.dll") → 替换 Storm.dll
            ├─ hook::injectdll(pi, "bin/LuaEngine.dll") → 代码洞注入
            └─ ResumeThread() → 恢复主线程
```

关键文件：
- `Development/Core/YDWE/Main.cpp` — YDWE 入口
- `Development/Core/YDWEStartup/LaunchWarcraft3.cpp` — 启动 war3.exe + 注入

### 2.2 代码洞注入机制

不使用 `CreateRemoteThread`（会被反外挂检测），而是**直接操作目标进程主线程的 EIP/RIP 寄存器**，让 shellcode 在主线程上下文中执行。

**x86 注入（injectdll_x86）：**

```
1. SuspendThread(hThread)
2. VirtualAllocEx → 为 shellcode 分配可执行内存
3. VirtualAllocEx → 为 DLL 路径字符串分配内存
4. 将 LoadLibraryW 地址、DLL 路径指针、原始 EIP 写入 shellcode
5. SetThreadContext → EIP = shellcode 地址
6. ResumeThread(hThread)

Shellcode 执行流程（39 字节）：
  push EIP(original)     ; 保存返回地址
  pushfd / pushad         ; 保存标志寄存器+通用寄存器
  push [DLL路径指针]
  call [LoadLibraryW]     ; 加载 LuaEngine.dll
  popad / popfd           ; 恢复寄存器
  ret                     ; 跳回原始 EIP 继续执行
```

**x64 注入（injectdll_x64）：**
- 通过 `wow64ext.h` 从 32 位进程操作 64 位进程
- 使用 `LdrLoadDll`（从 `ntdll.dll`）替代 `LoadLibraryW`
- shellcode 保存/恢复所有 x64 寄存器（push/pop r*）

**DLL 选择逻辑：**

```cpp
bool injectdll(PROCESS_INFORMATION& pi, wstring x86dll, wstring x64dll) {
    if (is_process64(pi.hProcess))
        return injectdll_x64(pi, x64dll);  // 64位war3 → 64位LuaEngine
    else
        return injectdll_x86(pi, x86dll);  // 32位war3 → 32位LuaEngine
}
```

关键文件：
- `Development/Core/ydbase/base/hook/injectdll.cpp` — 注入实现

---

## 三、第二阶段：LuaEngine.dll 启动与 Lua 引导系统

### 3.1 LuaEngine.dll 入口

当 `LoadLibrary("LuaEngine.dll")` 在 shellcode 中执行时，`DllMain` 被调用：

```cpp
// DllMain.cpp
BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        std::wstring name = getenv(L"ydwe-process-name"); // "war3"
        lua_State* L = LuaEngineCreate(name.c_str());
        if (L) {
            LuaEngineStart(L);  // require "main"
        }
    }
}
```

**LuaEngineCreate 做的事情：**
1. 调用 `FakeLuaPcall` **Hook `lua_pcallk`**，用 SEH（`__try/__except`）包裹所有 Lua 调用，防止 Lua 崩溃传播到 war3 进程
2. `luaL_newstate()` 创建 Lua 5.4 状态机
3. 打开所有标准库
4. 设置 `package.path` → `script/common/?.lua;script/war3/?.lua`
5. 设置 `package.cpath` → `ydwe/bin/?.dll`

**LuaEngineStart：**
```cpp
void LuaEngineStart(lua_State* L) {
    lua_pushcfunction(L, error_handler);
    lua_getglobal(L, "require");
    lua_pushstring(L, "main");
    lua_pcall(L, 1, 0, -3);  // require "main" → 加载 script/war3/main.lua
}
```

关键文件：
- `Development/Core/LuaEngine/DllMain.cpp`
- `Development/Core/LuaEngine/LuaEngine.cpp`

### 3.2 Lua 引导脚本：war3/main.lua

`script/war3/main.lua` 是整个插件系统的编排器，三个步骤：

**Step A — IAT Hook LoadLibraryA：**

拦截 `war3.exe` 对 `kernel32!LoadLibraryA` 的调用，当检测到正在加载 `Game.dll` 时：
- 可选加载补丁版 Game.dll
- 打开 `patch.mpq`
- 发布事件 `'GameDll加载'`（携带 Game.dll 模块句柄）

**Step B — 扫描加载所有插件 DLL：**

响应 `'GameDll加载'` 事件，三个并行处理器：
1. Hook `Game.dll` 的 `CreateWindowExA` → 检测魔兽窗口创建
2. 加载 `yd_loader.dll`（IAT Hook 加载器），挂载战争补丁
3. **扫描 `plugin/warcraft3/` 目录，逐个加载所有 `.dll`，并调用其 `Initialize()` 导出函数**

**Step C — 事件驱动初始化：**

后续各插件（yd_lua_engine、yd_jass_api 等）在各自的 `Initialize()` 中向引导系统注册回调，挂载到 `'GameDll加载'`、`'窗口初始化'`、`'游戏重置'` 等事件上，形成完整的事件驱动架构。每个插件只关心自己的初始化时机，无需感知其他插件的存在。

关键文件：
- `Development/Component/script/war3/main.lua`

---

## 四、第三阶段：JASS-Lua 桥接（yd_lua_engine.dll）

### 4.1 yd_lua_engine.dll 初始化

`yd_lua_engine.dll` 由 `war3/main.lua` 通过 `LoadLibraryW` + `GetProcAddress("Initialize")` 加载并调用。

`lua_loader::initialize()` 设置一个 **虚拟 MPQ 监视器** 来检测地图加载：

```cpp
void lua_loader::initialize() {
    // 监视 war3map.j 的访问 —— 这是地图的 JASS 脚本文件
    virtual_mpq::force_watch("war3map.j", [&](...) {
        if (地图没有变化) return false;
        initialize_lua();  // 初始化地图 Lua 环境
        return false;
    });

    // 监听游戏重置事件
    event_game_reset([&]() {
        lua_close(mainL);  // 销毁 Lua 状态
        mainL = nullptr;
    });
}
```

### 4.2 地图类型检测

`initialize_lua()` 决定当前地图使用哪种脚本模式：

```cpp
void initialize_lua() {
    if (地图MPQ中存在 "script\\war3map.lua") {
        // ====== 纯 Lua 模式 ======
        lua_State* L = getMainL();       // 创建地图 Lua 状态
        luaL_openlibs(L);                // 打开标准库
        open_lua_engine(L);              // 注册所有 jass.* 模块
        runtime::initialize();           // 初始化运行时配置
        luaL_loadbuffer(L, war3map.lua); // 加载地图 Lua 脚本
        safe_call(L, 0, 0, true);        // 执行
    } else {
        // ====== JASS + Lua 混合模式 ======
        // Hook Cheat 原生函数，注册 EXExecuteScript
        jass::table_hook("Cheat", &RealCheat, FakeCheat);
        jass::japi_table_add(EXExecuteScript, "EXExecuteScript", "(S)S");
    }
}
```

### 4.3 地图 Lua 状态的创建

```cpp
static lua_State* getMainL() {
    lua_State* L = newstate();   // Lua 5.4 + RNG 种子与 war3 同步
    luaL_openlibs(L);            // 标准库
    open_lua_engine(L);          // 注册 jass.* 模块
    runtime::initialize();       // 初始化运行时配置
    return L;
}
```

`newstate()` 在 `fix_baselib.cpp` 中实现：
- 创建 Lua 5.4 状态机
- 用 Warcraft III 内部随机种子初始化 `math.random`
- 应用安全加固（禁止 `dofile`, `loadfile`, `os.execute` 等）

### 4.4 jass.* 模块注册

`open_lua_engine()` 将以下模块以 preload 方式注册到 `package.preload` 中：

| 模块 | 功能 | 源文件 |
|---|---|---|
| `jass.common` | JASS 原生函数（Blizzard.j + Common.j） | `libs_common.cpp` |
| `jass.globals` | 读写 JASS 全局变量 | `libs_globals.cpp` |
| `jass.japi` | 从 Lua 注册新的 JASS 原生函数 | `libs_japi.cpp` |
| `jass.hook` | 从 Lua Hook JASS 函数 | `libs_hook.cpp` |
| `jass.runtime` | 运行时配置（handle_level, sleep, crash_catch 等） | `libs_runtime.cpp` |
| `jass.slk` | SLK 表访问 | `libs_slk.cpp` |
| `jass.console` | 调试控制台 | `libs_console.cpp` |
| `jass.debug` | 调试器支持 | `libs_debug.cpp` |
| `jass.message` | 消息传递 | `libs_message.cpp` |
| `jass.bignum` | 大数运算 | `libs_bignum.cpp` |

同时初始化 JASS 类型元表：
- `jhandle_ud_make_mt` — handle userdata 元表（`__eq`, `__tostring`, `__gc`）
- `jhandle_lud_make_mt` — handle lightuserdata 元表
- `jarray_make_mt` — JASS 数组元表
- `jreal::init` — real 数值元表

### 4.5 安全加固

`fix_baselib.cpp` 对 Lua 标准库做了大量限制（沙箱）：

```
禁用函数：
  dofile, loadfile          — 禁止从文件系统加载代码
  os.execute, os.exit        — 禁止执行外部命令、退出进程
  os.getenv                  — 禁止读取环境变量
  os.remove, os.rename       — 禁止操作文件系统
  io.popen, io.lines         — 禁止管道执行
  io.tmpfile                 — 禁止临时文件
  io.input, io.output        — 禁止重定向标准IO
  io.stdin, io.stdout, io.stderr — 禁止直接访问标准流

文件IO重定向：
  所有文件IO通过 Storm.dll（MPQ 接口）进行
  写入限制在 Warcraft III 安装目录内
  禁止写入 .exe, .dll, .asi, .m3a 等可执行文件类型
```

---

## 五、第四阶段：双虚拟机运行时架构

### 5.1 两个独立的 Lua 状态

系统运行 **两个完全独立的 Lua 状态机**：

| 状态机 | 创建者 | 用途 |
|---|---|---|
| **LuaEngine 状态** | `LuaEngine.dll` → `DllMain` | 运行插件管理系统 (`war3/main.lua`) — IAT Hook、DLL 加载、窗口管理、事件分发 |
| **地图 Lua 状态** | `yd_lua_engine.dll` → `getMainL()` | 运行地图脚本 (`war3map.lua`) — 游戏逻辑、JASS 互操作 |

两者的角色：
- **LuaEngine 状态**（先创建）：常驻，负责基础设施（`ydwe-process-name = "war3"` 标识进程身份）
- **地图 Lua 状态**（懒加载）：在 `war3map.j` 被访问时创建，随地图重置而销毁重建

### 5.2 JASS VM 结构

JASS 虚拟机在 Game.dll 中运行，其内部结构通过指针偏移访问：

```cpp
struct jass_vm_t {
    char        unk0[0x20];
    opcode*     opcode;          // 0x20 - 当前指令指针
    // ...
    uint32_t    has_sleep;       // 0x34 - sleep 标记
    // ...
    uint32_t    index;           // 0x2850 - VM 索引
    variable_table* global_table;  // 0x285C - 全局变量表
    stackframe_t*   stackframe;    // 0x2868 - 调用栈
    string_fasttable* string_table; // 0x2874 - 字符串表
    code_table_t*   code_table;    // 0x2888 - 函数/代码表
    handle_table_t** handle_table; // 0x28A4 - handle 引用表
};
```

这些偏移量通过 `war3_searcher`（模式匹配）在运行时定位，支持多版本适配。

关键文件：
- `Development/Core/ydwar3/warcraft3/hashtable.h`

### 5.3 核心互操作机制

#### A) Lua 调用 JASS 原生函数

当 Lua 代码写 `local t = CJ.CreateTrigger()` 时：

1. `jass.common` 表的 `__index` 元方法被触发
2. `jass_get(L)` 在 JASS 函数表中查找 `CreateTrigger`
3. 找到后返回一个 **C closure**，upvalue 是 `jass::func_value*` 指针
4. 调用时进入 `jass_call_closure(L)`：
   ```
   jass_call_closure(L)
     └─ jass_call_native_function(L, nf, func_address)
          ├─ 从 Lua 栈读取参数，通过 jassbind 转换为 JASS 类型
          │    boolean → TYPE_BOOLEAN
          │    number  → TYPE_INTEGER / TYPE_REAL
          │    string  → TYPE_STRING (通过 jass::create_string)
          │    userdata/lightuserdata → TYPE_HANDLE
          │    function → TYPE_CODE (通过 trampoline)
          ├─ 构造 jass::call_param 参数列表
          ├─ [可选] SEH 包裹：__try { jass::call(...) } __except { ... }
          └─ 将返回值通过 jass_push 转换回 Lua 类型
   ```

关键文件：
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/common.cpp`
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/jassbind.cpp`

#### B) Lua 调用 JASS（Sleep 感知）

`jass_call_closure` 有一个关键的分支，处理 `TriggerSleepAction` 等会挂起 JASS 执行的函数：

```cpp
int jass_call_closure(lua_State* L) {
    int result = jass_call_native_function(L, ...);
    if (lua_isyieldable(L)) {
        jass_vm_t* thread = get_jass_thread();
        if (thread && thread->has_sleep) {
            // 回退 JASS 指令指针（跳过 trampoline 占用的 9 条 opcode）
            thread->opcode -= jass::trampoline_size() / sizeof(jass::opcode);
            return lua_yield(L, 0);  // Lua 协程让出执行权
        }
    }
    return result;
}
```

当检测到 JASS 执行了 sleep 类函数后：
1. 回退 JASS 的 opcode 指针（因为 JASS 会在 sleep 后重新执行 trampoline）
2. Lua 协程 `yield`，让出执行权
3. JASS 继续执行后续 opcode
4. sleep 结束后 JASS 重新进入 trampoline → 恢复 Lua 协程

#### C) JASS 调用 Lua 函数（Trampoline 机制）

当 Lua 将函数作为 `code` 类型参数传给 JASS 时（如 `TriggerAddAction`），需要创建一个 JASS 可调用的入口。

**Trampoline 创建：**

```
cfunction_to_code(L, index)
  ├─ runtime::callback_push(L, index)  → 将 Lua 函数注册到回调表，获得整数引用 ref
  ├─ jass::trampoline_create(callback, param1, param2)
  │    ├─ 分配可执行内存
  │    ├─ 生成 9 条 JASS opcode（跳板字节码）
  │    └─ 返回 opcode 地址（即 JASS 中的 code 值）
  └─ 返回 code handle
```

**Trampoline 汇编存根（`lua_to_nativefunction.h`）：**

```asm
push esi              ; 保存 esi
mov  esi, esp
add  esi, 8           ; esi = 参数列表指针
push esi              ; 将参数列表地址压栈
mov  ecx, this        ; 设置 this 指针
call lua_function     ; 调用 C++ 方法
pop  esi              ; 恢复 esi
ret
```

这个 stub 符合 JASS 的调用约定（`__stdcall`-like，参数指针在 ESI），可以被直接放在 JASS 原生函数表中使用。

**回调触发：**

```
JASS VM 执行到 trampoline opcode
  └─ jass_callback(lua_State* L, uint32_t ref)
       └─ safe_call_ref(L, ref, nargs, result_vt)
            ├─ runtime::callback_read(L, ref)  → 从注册表取出 Lua 函数
            ├─ 从 JASS 参数列表读取参数并压入 Lua 栈
            ├─ safe_call(L, nargs, nresults, true) → 调用 Lua 函数
            └─ 将返回值转换回 JASS 类型
```

#### D) JASS 调用 Lua（EXExecuteScript）

`EXExecuteScript` 是一个 JAPI 函数，允许 JASS 代码直接执行 Lua 表达式：

```jass
// JASS 侧：
local string result = EXExecuteScript("some_lua_function(42)")

// Lua 侧：
function some_lua_function(x)
    return tostring(x * 2)
end
```

实现：

```cpp
jstring_t EXExecuteScript(jstring_t script) {
    lua_State* L = getMainL();
    std::string str = fmt::format("return ({})", from_string(script));
    luaL_loadbuffer(L, str.c_str(), str.size(), ...);
    safe_call(L, 0, 1, true);
    // 返回 Lua 表达式结果（如果是字符串）
    jstring_t result = 0;
    if (lua_isstring(L, -1))
        result = jass::create_string(lua_tostring(L, -1));
    lua_pop(L, 1);
    return result;
}
```

#### E) Lua 通过 cheat 命令执行代码

`FakeCheat` Hook 了 JASS 的 `Cheat` 原生函数。当玩家在聊天中输入 `exec-lua:"模块名"` 时：

```
输入：exec-lua:"mylib"
  → FakeCheat 截获
  → require "mylib"
  → 执行对应 Lua 模块
```

#### F) Lua Hook JASS 函数

`jass.hook` 模块允许从 Lua 侧拦截任意 JASS 函数：

```lua
local hook = require "jass.hook"
hook["CreateUnit"] = function(jass_func, id, x, y, face)
    print("CreateUnit called:", id, x, y)
    return jass_func(id, x, y, face)  -- 调用原函数
end
```

底层通过 `jass::hook()` 修改 JASS 原生函数表，将函数地址替换为动态生成的 `lua_to_nativefunction` 汇编存根。存根在调用 Lua 钩子后，可以选择继续调用原始 JASS 函数。

### 5.4 并发执行模型

双虚拟机的"并行"实际上是**协作式并发**，而非真正的多线程并行：

```
                    JASS VM（主循环）                    Lua VM
                    ================                    ======

war3 主循环 → 处理事件 → 遇到 Lua trampoline → 进入 Lua 执行
                                                    │
                                                    ├─ jass.common 调用 JASS 原生函数
                                                    │   └─ jass::call() 直接调用原生函数地址
                                                    │       （在 JASS VM 上下文执行，类似函数调用）
                                                    │
                                                    ├─ 遇到 sleep → lua_yield
                                                    │   └─ 回退 JASS opcode 指针
                                                    │   └─ 控制权交还 JASS VM
                                                    │
                                                    └─ Lua 脚本执行完毕 → 返回 JASS VM

JASS 执行剩余 opcode → sleep 结束 → 重新进入 trampoline
  └─ trampoline 恢复 Lua 协程（lua_resume）
      └─ Lua 从 yield 点继续执行
```

关键设计点：
1. **Lua 调用 JASS 是同步的** — `jass::call()` 直接调用原生函数地址，在同一个调用栈中执行，等同于一次 C 函数调用
2. **JASS 调用 Lua 通过 trampoline** — 创建一个 JASS opcode 序列作为桥梁，JASS 执行到此时进入 C++ 上下文，再进入 Lua
3. **Sleep 的中断机制** — 当 Lua 调用了会 sleep 的 JASS 函数后，Lua 协程 yield，JASS 继续运行，sleep 结束后通过 trampoline 恢复 Lua 协程
4. **两个 Lua 状态独立运行** — LuaEngine 状态处理插件和窗口管理，地图 Lua 状态处理游戏逻辑，两者通过事件系统交互

### 5.5 Handle 管理

JASS 的 handle（对象引用）在 Lua 中有三种表示级别：

| 级别 | 实现 | 安全性 | 性能 |
|---|---|---|---|
| Level 0 | 普通整数 | 无保护，handle 可能失效 | 最快 |
| Level 1 | lightuserdata | 只做相等比较 | 快 |
| Level 2（默认） | 全 userdata + `__gc` | 引用计数，GC 时自动释放 JASS handle | 有开销 |

默认 Level 2 下：
- 创建 handle 时调用 `handle_ref()` 增加引用
- 垃圾回收时调用 `handle_unref()` 释放引用
- 通过 `jass.runtime.handle_level = 0|1|2` 切换级别

---

## 六、完整流程时序图

```
YDWE.exe 启动
  │
  ├─ 加载 YDWEStartup.dll
  │
  └─ YDWEStartup()
       │
       ├─ CreateProcess("war3.exe", ..., CREATE_SUSPENDED)
       │    └─ 进程创建，主线程挂起
       │
       ├─ injectdll(pi, "LuaEngine.dll")
       │    ├─ VirtualAllocEx → shellcode 内存
       │    ├─ VirtualAllocEx → DLL路径字符串内存
       │    ├─ 写入 LoadLibraryW 地址、DLL路径、原始EIP
       │    ├─ SetThreadContext → EIP = shellcode
       │    └─ ResumeThread(hThread)
       │         │
       │         ├─ [shellcode 执行]
       │         ├─ LoadLibraryW("LuaEngine.dll")
       │         │    └─ DllMain(DLL_PROCESS_ATTACH)
       │         │         ├─ getenv("ydwe-process-name") → "war3"
       │         │         ├─ LuaEngineCreate("war3")
       │         │         │    ├─ Hook lua_pcallk (SEH wrapper)
       │         │         │    ├─ luaL_newstate() → Lua 5.4
       │         │         │    └─ luaL_openlibs(L)
       │         │         │
       │         │         └─ LuaEngineStart(L)
       │         │              └─ require "main"
       │         │                   └─ war3/main.lua 执行
       │         │                        │
       │         │                        ├─ IAT Hook: kernel32!LoadLibraryA
       │         │                        │
       │         ├─ [shellcode ret 回原始 EIP]
       │         │
       │         ├─ [war3.exe 正常启动...]
       │         │
       │         ├─ LoadLibraryA("Game.dll")
       │         │    └─ [被 Hook 截获]
       │         │         ├─ 发布时间 'GameDll加载'
       │         │         └─ 事件处理器触发：
       │         │              │
       │         │              ├─ Hook Game.dll!CreateWindowExA → 窗口检测
       │         │              ├─ LoadLibrary("yd_loader.dll") → IAT Hook
       │         │              └─ 扫描 plugin/warcraft3/*.dll:
       │         │                   ├─ LoadLibrary("yd_lua_engine.dll")
       │         │                   │    └─ GetProcAddress("Initialize")
       │         │                   │         └─ lua_loader::initialize()
       │         │                   │              ├─ virtual_mpq::watch("war3map.j")
       │         │                   │              └─ event_game_reset(handler)
       │         │                   │
       │         │                   ├─ LoadLibrary("yd_jass_api.dll")
       │         │                   │    └─ Initialize()
       │         │                   │         └─ 注册 UnitState, AbilityState 等 JAPI 扩展
       │         │                   │
       │         │                   └─ [其他插件...]
       │         │
       │         ├─ CreateWindowExA → 魔兽窗口创建
       │         │    └─ [被 Hook 截获]
       │         │         └─ 发布 '窗口初始化' → 自动进局等处理
       │         │
       │         └─ 加载地图 → 访问 war3map.j
       │              └─ [virtual_mpq 监视器触发]
       │                   └─ initialize_lua()
       │                        │
       │                        ├─ [存在 war3map.lua]
       │                        │    ├─ getMainL() → 创建地图 Lua 状态
       │                        │    ├─ open_lua_engine(L) → 注册 jass.* 模块
       │                        │    ├─ load war3map.lua
       │                        │    └─ safe_call → 执行地图 Lua 脚本
       │                        │
       │                        └─ [不存在 war3map.lua]
       │                             ├─ Hook Cheat → exec-lua: 支持
       │                             └─ 注册 EXExecuteScript → JASS→Lua 调用
       │
       └─ [war3 进入游戏主循环]
            │
            └─ [双虚拟机并存运行]
                 ├─ JASS VM 主循环处理 opcode
                 │    ├─ 遇到 Lua trampoline → 进入 Lua 执行
                 │    │    ├─ Lua 调用 jass.common → jass_call_native_function
                 │    │    ├─ Lua yield (sleep) → 控制权还 JASS
                 │    │    └─ Lua 脚本结束 → 返回 JASS
                 │    └─ 处理其它 opcode...
                 │
                 └─ Lua VM (事件驱动)
                      ├─ 地图 game events → Lua 回调
                      ├─ timer/tick → Lua 函数
                      └─ 控制台输入 exec-lua: → require 模块
```

---

## 七、关键技术点总结

| 技术点 | 实现方式 |
|---|---|
| **进程注入** | 代码洞注入（Code Cave），通过修改 EIP/RIP 在主线程上下文执行 shellcode |
| **Hook 加载** | IAT Hook（`kernel32!LoadLibraryA`），在 Game.dll 加载时机注入插件 |
| **双 Lua 状态** | LuaEngine 状态（插件系统）+ 地图 Lua 状态（游戏逻辑），独立运行 |
| **JASS→Lua 调用** | Trampoline（9 条 JASS opcode） + `jass_callback` + `safe_call_ref` |
| **Lua→JASS 调用** | C closure + `jass_call_native_function` + `jass::call()` 直接调用原生函数 |
| **Sleep 处理** | Lua coroutine yield + JASS opcode 回退 + trampoline 恢复 |
| **类型转换** | `jassbind` 模块统一处理六种 JASS 类型（nothing, boolean, integer, real, string, handle, code） |
| **Handle 管理** | 三级可选：整数/lightuserdata/userdata，默认 userdata + GC 自动释放 |
| **安全加固** | SEH 包裹 pcall、沙箱标准库、MPQ 重定向文件 IO、禁止写入可执行文件 |
| **版本适配** | `war3_searcher` 通过模式匹配定位 JASS VM 内部结构偏移 |

---

## 八、核心源文件索引

**注入与启动：**
- `Development/Core/YDWEStartup/LaunchWarcraft3.cpp` — 创建 war3.exe 进程 + 注入 LuaEngine.dll
- `Development/Core/ydbase/base/hook/injectdll.cpp` — 代码洞注入实现（x86/x64）

**核心 Lua 运行时：**
- `Development/Core/LuaEngine/DllMain.cpp` — LuaEngine.dll 入口
- `Development/Core/LuaEngine/LuaEngine.cpp` — 创建 Lua 状态、SEH pcall 包装

**JASS-Lua 桥接：**
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/lua_loader.cpp` — 桥接入口：地图检测、双模式选择
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/common.cpp` — Lua→JASS 调用实现
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/callback.cpp` — JASS→Lua 回调、trampoline 创建、safe_call
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/lua_to_nativefunction.h` — 动态汇编存根生成
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/open_lua_engine.cpp` — jass.* 模块注册
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/libs_common.cpp` — jass.common（JASS 函数分派）
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/libs_hook.cpp` — jass.hook（Hook JASS 函数）
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/libs_japi.cpp` — jass.japi（注册 JAPI 函数）
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/libs_runtime.cpp` — 运行时配置
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/fix_baselib.cpp` — 安全加固、RNG 同步
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/jassbind.cpp` — JASS↔Lua 类型转换
- `Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/class_handle.cpp` — Handle 管理

**JASS VM 底层：**
- `Development/Core/ydwar3/warcraft3/hashtable.h` — JASS VM 结构体定义
- `Development/Core/ydwar3/warcraft3/jass.h` — JASS 类型、调用约定
- `Development/Core/ydwar3/warcraft3/jass/trampoline_function.h` — Trampoline 创建
- `Development/Core/ydwar3/warcraft3/jass/hook.h` — JASS Hook API

**Lua 引导脚本：**
- `Development/Component/script/war3/main.lua` — 主引导脚本（IAT Hook + 插件加载）

**插件加载器：**
- `Development/Plugin/Warcraft3/yd_loader/DllMain.cpp` — yd_loader 入口
- `Development/Plugin/Warcraft3/yd_loader/DllModule.cpp` — IAT Hook 实现
