# YDWE 核心提取 & Callback 适配 — 详细设计方案

## 1. 背景

当前项目需要基于 YDWE 核心源码编译适配的 DLL 链，通过 callback exploit 在 MPQ 模式下加载。YDWE 的 DLL 原本通过 hook Storm.dll 的 `StormAlloc` 函数来检测 JASS VM 初始化时刻，但在 callback 模式下，VM 已经初始化完毕（callback 在 `war3map.j::main()` 中触发），导致 YDWE 的事件驱动机制无法触发。

本方案的目标是从 YDWE 源码中提取核心模块，改编为适配 callback 时序的版本。

## 2. YDWE 原始时序 vs Callback 时序

### 2.1 YDWE 原始时序（Storm.dll hook）

```
Game.dll 加载
  → Storm.dll 加载地图
    → nf_register.cpp hook StormAlloc
      → 检测到 (amount=176, log_line=668, filename="Jass.cpp")
        → stat = 1, 记录 thread_id
      → 检测到 TlsGetValue 调用（stat 1→2→3）
        → event_add() → nfunction_add()  ← 注册新 native
      → 检测到 (amount=140, log_line=90, filename="Agile.cpp")
        → event_hook() → nfunction_hook()  ← hook 已有 native
    → war3map.j 加载 → main() 执行
    → war3map.lua 加载 → Lua 脚本执行
```

**关键：DLL 在 VM 初始化之前就位，通过 StormAlloc 回调被动感知 VM 状态。**

### 2.2 Callback 时序

```
Game.dll 加载
  → Storm.dll 加载地图
  → JASS VM 初始化（无 hook，静默完成）
  → war3map.j 加载 → main() 执行
    → StartCampaignAI(Player(12), "callback")
    → callback exploit → 内存读写 → VirtualAlloc
    → ExportFileFromMpq + LoadLibrary
      → 加载 lua53.dll
      → 加载 ydbase.dll
      → 加载 ydwar3.dll
      → 加载 yd_lua_engine.dll → Initialize()
      → 加载 yd_jass_api.dll → Initialize()
    → callback 继续执行...
```

**关键：DLL 在 VM 初始化之后加载，StormAlloc hook 不会触发（VM 已经完成初始化）。**

## 3. 需要提取的 YDWE 模块

### 3.1 模块依赖图

```
yd_core/（从 ydwar3 提取）
├── hashtable.h           ← warcraft3/hashtable.h（VM 内存布局）
├── jass.h/cpp            ← warcraft3/jass.h + jass.cpp（类型、字符串、调用）
├── func_value.h/cpp      ← warcraft3/jass/func_value.h + .cpp（native 扫描 & 注册）
├── hook.h/cpp            ← warcraft3/jass/hook.h + .cpp（hook 框架）
├── nf_register.h/cpp     ← warcraft3/jass/nf_register.h + .cpp（★ 需大幅改编）
├── war3_searcher.h/cpp   ← warcraft3/war3_searcher.h + .cpp（版本检测）
├── string_hash.h         ← warcraft3/detail/string_hash.h（哈希算法）
├── version.h             ← warcraft3/version.h（版本枚举）
├── config.h              ← warcraft3/config.h（导出宏）
├── opcode.h/cpp          ← warcraft3/jass/opcode.h + .cpp（字节码操作）
└── event.h/cpp           ← warcraft3/event.h + .cpp（事件系统）

lua_engine/（从 yd_lua_engine 提取）
├── lua_loader.h/cpp      ← lua_engine/lua_loader.h + .cpp（★ 需大幅改编）
├── open_lua_engine.h/cpp ← lua_engine/open_lua_engine.h + .cpp（直接复用）
├── jassbind.h/cpp        ← lua_engine/jassbind.h + .cpp（Lua-JASS 绑定）
├── callback.h/cpp        ← lua_engine/callback.h + .cpp（安全调用）
├── class_real.h/cpp      ← lua_engine/class_real.h + .cpp（real 类型）
├── class_handle.h/cpp    ← lua_engine/class_handle.h + .cpp（handle 类型）
├── class_array.h/cpp     ← lua_engine/class_array.h + .cpp（array 类型）
├── fix_baselib.h/cpp     ← lua_engine/fix_baselib.h + .cpp（Lua 基础库修复）
├── storm.h/cpp           ← lua_engine/storm.h + .cpp（MPQ 文件访问）
├── common.h/cpp          ← lua_engine/common.h + .cpp（共享工具）
├── libs_common.cpp       ← jass.common 模块
├── libs_globals.cpp      ← jass.globals 模块
├── libs_hook.cpp         ← jass.hook 模块
├── libs_japi.cpp         ← jass.japi 模块
├── libs_runtime.cpp      ← jass.runtime 模块
├── libs_slk.cpp          ← jass.slk 模块
├── libs_console.cpp      ← jass.console 模块
├── libs_debug.cpp        ← jass.debug 模块
├── libs_message.cpp      ← jass.message 模块
├── libs_bignum.cpp       ← jass.bignum 模块
├── libs_log.cpp          ← jass.log 模块
├── libs_ai.cpp           ← jass.ai 模块
└── libs_selector.cpp     ← jass.selector 模块

extensions/（自定义扩展）
├── dllmain.cpp           ← DLL 入口，Initialize() 导出
└── custom_natives.cpp    ← 自定义 native 注册
```

### 3.2 外部依赖

| 依赖 | 来源 | 说明 |
|------|------|------|
| base/hook/* | ydbase | IAT/EAT/inline hook 框架 |
| base/util/* | ydbase | signal、do_once、singleton 等工具 |
| base/hook/fp_call.h | ydbase | 函数调用封装（std_call、c_call、fast_call） |
| bee.lua / lua54 | OpenSource/bee.lua | Lua 5.4 运行时 |
| fmt | yd_lua_engine | 格式化库（EXExecuteScript 用） |

## 4. 需要改编的模块（核心）

### 4.1 nf_register.cpp — 改编方案

**原始实现（193 行）：**
- hook `Storm.dll` ordinal 401（StormAlloc）和 `Kernel32.dll` TlsGetValue
- 通过检测 StormAlloc 的参数模式（amount=176, log_line=668, filename="Jass.cpp"）判断 VM 初始化
- 通过 TlsGetValue 的调用次数（stat 1→2→3）精确触发 `nfunction_add()`
- 有 4 个版本的 fake_storm_alloc（120、122、126、127+），对应不同 WC3 版本的日志路径

**Callback 模式的问题：**
- VM 已经初始化完毕，StormAlloc 的那些特定调用不会再发生
- hook 永远不会触发 → `nfunction_add()` 永远不会被调用

**改编方案：删除 StormAlloc hook，改为直接初始化**

```cpp
// nf_register.cpp — callback 适配版
namespace warcraft3::jass::nf_register {
    base::signal<void, void> event_hook;
    base::signal<void, void> event_add;

    bool initialize()
    {
        // 不再 hook StormAlloc
        // 直接触发 event_add 和 event_hook
        // 因为 callback 模式下 DLL 加载时 VM 已经就绪

        event_add();
        nfunction_add();

        event_hook();
        nfunction_hook();

        return true;
    }
}
```

**关键改动：**
1. 删除所有 `fake_storm_alloc*` 函数
2. 删除 `fake_tls_get_value` 函数
3. 删除 `real_storm_alloc`、`real_tls_get_value` 等全局变量
4. `initialize()` 直接调用 `nfunction_add()` 和 `nfunction_hook()`
5. 保留 `event_add` 和 `event_hook` 信号（供外部监听）

### 4.2 lua_loader.cpp — 改编方案

**原始实现（142 行）：**
- 通过 `virtual_mpq::force_watch("war3map.j", ...)` 监听地图加载
- 当检测到新地图时，调用 `initialize_lua()` 创建 Lua 状态
- `initialize_lua()` 从 MPQ 加载 `script\war3map.lua`
- 如果 war3map.lua 不存在，hook `Cheat` native 支持 `exec-lua:` 命令
- 注册 `EXExecuteScript` JAPI 函数
- 通过 `event_game_reset` 监听游戏重置，关闭 Lua 状态

**Callback 模式的问题：**
- `virtual_mpq::force_watch("war3map.j", ...)` 在 YDWE 正常流程中，DLL 在地图加载前就位，所以能拦截 war3map.j 的读取
- 在 callback 模式下，war3map.j 已经执行完毕，这个 hook 不会触发

**改编方案：Initialize() 里主动创建 Lua 状态并加载 war3map.lua**

```cpp
// lua_loader.cpp — callback 适配版
namespace warcraft3::lua_engine::lua_loader {
    static lua_State* mainL = 0;

    // 保持不变：getMainL() 创建 Lua 状态
    static lua_State* getMainL()
    {
        if (!mainL) {
            lua_State* L = newstate();
            if (L) {
                luaL_openlibs(L);
                open_lua_engine(L);
                runtime::initialize();
            }
            mainL = L;
        }
        return mainL;
    }

    // 保持不变：EXExecuteScript
    jass::jstring_t __cdecl EXExecuteScript(jass::jstring_t script) { ... }

    // 改编：不再依赖 virtual_mpq::force_watch
    void initialize()
    {
        // 直接创建 Lua 状态并加载 war3map.lua
        lua_State* L = getMainL();
        if (!L) return;

        // 从 MPQ 加载 war3map.lua
        const char* buf = 0;
        size_t len = 0;
        if (storm_s::instance().load_file("script\\war3map.lua", (const void**)&buf, &len))
        {
            if (luaL_loadbuffer(L, buf, len, "@script\\war3map.lua") != LUA_OK) {
                printf("%s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
                storm_s::instance().unload_file(buf);
                return;
            }
            safe_call(L, 0, 0, true);
            storm_s::instance().unload_file(buf);
        }

        // 注册 EXExecuteScript（即使 war3map.lua 不存在也要注册）
        jass::japi_table_add((uintptr_t)EXExecuteScript, "EXExecuteScript", "(S)S");

        // hook Cheat 支持 exec-lua: 命令
        // （RealCheat/FakeCheat 逻辑保持不变）
    }
}
```

**关键改动：**
1. 删除 `virtual_mpq::force_watch("war3map.j", ...)` 监听
2. 删除 `event_game_reset` 监听（callback 模式下不需要重置处理）
3. `initialize()` 直接调用 `getMainL()` + `initialize_lua()`
4. 保持 `EXExecuteScript` 和 `FakeCheat` 不变

### 4.3 hook.cpp — 小幅调整

**原始实现（346 行）：**

`hook.cpp` 的核心逻辑是：
- `japi_add()` → 将 native 信息存入 `add_info_list`
- `japi_hook()` → 将 hook 信息存入 `hook_info_list`
- `nfunction_add()` → 遍历 `add_info_list`，调用 `japi_table_add` 注册
- `nfunction_hook()` → 遍历 `hook_info_list`，调用 `table_hook` 注册
- `japi_table_add()` → 搜索 `nfunction_add` 函数地址，调用它注册 native
- `detail::async_initialize()` → 调用 `nf_register::initialize()`

**Callback 模式的影响：**
- `japi_table_add()` 中的 `detail::search_register_func()` 搜索 "StringCase" 字符串来定位 `nfunction_add` 函数，这个在 callback 模式下仍然有效（game.dll 的 .text 段没变）
- `detail::async_initialize()` 调用 `nf_register::initialize()`，改编后会直接触发

**改编方案：基本不需要改动**

`hook.cpp` 的逻辑在 callback 模式下仍然成立：
1. `japi_add()` 存入列表
2. `nf_register::initialize()`（改编后直接触发）调用 `nfunction_add()`
3. `nfunction_add()` 遍历列表调用 `japi_table_add()`
4. `japi_table_add()` 通过 `search_register_func()` 找到 `nfunction_add` 并调用

唯一需要注意的是调用顺序：必须先调用 `japi_add()` / `japi_hook()` 注册信息，再调用 `nf_register::initialize()` 触发注册。在 callback 模式下，`yd_lua_engine::Initialize()` 和 `yd_jass_api::Initialize()` 的调用顺序需要保证。

### 4.4 func_value.cpp — 不需要改动

`func_value.cpp` 的 `initialize_mapping("Deg2Rad")` 通过扫描 game.dll 的 .text 段来发现所有原生 JASS native。这个机制在 callback 模式下完全有效，因为 game.dll 的代码段不会因为加载时机不同而改变。

### 4.5 jass.cpp — 不需要改动

`jass.cpp` 提供的 `string_fake`、`from_string`、`create_string`、`call` 等函数都是直接操作 JASS VM 内存，不依赖加载时机。

## 5. DLL 加载顺序

callback 中的加载顺序必须严格保证：

```
1. lua53.dll          → LoadLibrary（Lua 运行时）
2. ydbase.dll         → LoadLibrary（hook 框架）
3. ydwar3.dll         → LoadLibrary（JASS VM 接口）
   └─ nf_register::initialize() 在首次被调用时触发
      → 但此时还没有注册任何 japi_add/japi_hook
      → 所以 nfunction_add() 和 nfunction_hook() 都是空操作
4. yd_lua_engine.dll  → LoadLibrary → Initialize()
   └─ lua_loader::initialize()
      → 创建 Lua 状态
      → open_lua_engine() 注册 jass.* 模块
      → 加载 war3map.lua
      → 注册 EXExecuteScript
   └─ jass::japi_add("EXExecuteScript", ...)
      → 首次调用触发 nf_register::initialize()
      → nfunction_add() 注册 EXExecuteScript
5. yd_jass_api.dll    → LoadLibrary → Initialize()
   └─ InitializeUnitState() 等
      → japi_hook("GetUnitState", ...) 等
      → japi_add("EXGetUnitString", ...) 等
      → nf_register::initialize() 已经触发过，不再重复
      → 但 japi_hook/japi_add 的信息已经存入列表
      → 需要手动调用 nfunction_add() 和 nfunction_hook()
```

**问题：** `nf_register::initialize()` 使用 `DO_ONCE_NOTHREADSAFE()` 保证只执行一次。但在 callback 模式下，第一次调用时 `add_info_list` 和 `hook_info_list` 可能还是空的。

**解决方案：** 在 `nf_register::initialize()` 中不使用 DO_ONCE，改为暴露一个 `flush()` 函数，在所有 DLL 加载完毕后手动调用。

```cpp
// nf_register.cpp — callback 适配版（修订）
namespace warcraft3::jass::nf_register {
    base::signal<void, void> event_hook;
    base::signal<void, void> event_add;
    static bool initialized = false;

    bool initialize()
    {
        if (initialized) return false;
        initialized = true;

        event_add();
        nfunction_add();

        event_hook();
        nfunction_hook();

        return true;
    }

    // 新增：允许外部触发重新注册
    void flush()
    {
        nfunction_add();
        nfunction_hook();
    }
}
```

或者更简单的方案：**在 callback 中，所有 DLL 加载完毕后，手动调用一次 `flush()`。**

## 6. 完整的 Callback 加载流程

```
callback::main():
  ┌─ 版本检测（1.27a / 1.26a / 1.24b / 1.24e）
  ├─ UnlockMemory → 任意内存读写
  ├─ AllocateExecutableMemory → RWX 内存
  │
  ├─ ExportFileFromMpq("lua53.dll") → LoadLibrary
  ├─ ExportFileFromMpq("ydbase.dll") → LoadLibrary
  ├─ ExportFileFromMpq("ydwar3.dll") → LoadLibrary
  │
  ├─ ExportFileFromMpq("yd_lua_engine.dll") → LoadLibrary
  │   └─ DllMain → 注册 japi_add / japi_hook 信息
  │
  ├─ ExportFileFromMpq("yd_jass_api.dll") → LoadLibrary
  │   └─ DllMain → 注册更多 japi_add / japi_hook 信息
  │
  ├─ ExportFileFromMpq("my_extension.dll") → LoadLibrary
  │   └─ DllMain → 注册自定义 japi_add 信息
  │
  └─ 调用 Initialize() 导出函数
      └─ nf_register::initialize()  ← 在这里触发，此时所有 japi_add/japi_hook 都已注册
          → nfunction_add()  ← 注册所有新 native
          → nfunction_hook() ← hook 所有已有 native
      └─ lua_loader::initialize()
          → 创建 Lua 状态
          → open_lua_engine()
          → 加载 war3map.lua
```

## 7. 文件清单 & 工作量估算

| 文件 | 来源 | 改编程度 | 工作量 |
|------|------|----------|--------|
| `yd_core/hashtable.h` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/string_hash.h` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/version.h` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/config.h` | ydwar3 | 适配导出宏 | 0.5h |
| `yd_core/jass.h` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/jass.cpp` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/opcode.h/cpp` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/func_value.h/cpp` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/hook.h/cpp` | ydwar3 | 小幅调整 | 1h |
| `yd_core/nf_register.h/cpp` | ydwar3 | **大幅改编** | 2h |
| `yd_core/war3_searcher.h/cpp` | ydwar3 | 原样复制 | 0.5h |
| `yd_core/event.h/cpp` | ydwar3 | 原样复制 | 0.5h |
| `lua_engine/lua_loader.h/cpp` | yd_lua_engine | **大幅改编** | 2h |
| `lua_engine/open_lua_engine.h/cpp` | yd_lua_engine | 原样复制 | 0.5h |
| `lua_engine/jassbind.h/cpp` | yd_lua_engine | 原样复制 | 0.5h |
| `lua_engine/callback.h/cpp` | yd_lua_engine | 原样复制 | 0.5h |
| `lua_engine/class_*.h/cpp` | yd_lua_engine | 原样复制 | 1h |
| `lua_engine/fix_baselib.h/cpp` | yd_lua_engine | 原样复制 | 0.5h |
| `lua_engine/storm.h/cpp` | yd_lua_engine | 原样复制 | 0.5h |
| `lua_engine/common.h/cpp` | yd_lua_engine | 原样复制 | 0.5h |
| `lua_engine/libs_*.cpp` (13个) | yd_lua_engine | 原样复制 | 2h |
| `extensions/dllmain.cpp` | 新建 | — | 1h |
| `extensions/custom_natives.cpp` | 新建 | — | 1h |
| `CMakeLists.txt` | 新建 | — | 2h |
| `build.bat` | 适配 | — | 1h |
| **总计** | | | **~20h** |

## 8. 风险 & 注意事项

### 8.1 Lua 版本差异
YDWE 使用 **Lua 5.4**（通过 bee.lua），而当前项目使用 **Lua 5.3**。需要决定：
- **方案 A**：跟随 YDWE 用 Lua 5.4（需要更新 vendor/lua 源码）
- **方案 B**：保持 Lua 5.3（需要检查 yd_lua_engine 的代码是否兼容 5.3）

建议用方案 A，因为 YDWE 的 libs_*.cpp 可能使用了 5.4 特有的 API。

### 8.2 bee.lua 依赖
yd_lua_engine 依赖 bee.lua 库（Unicode 工具、Lua 包管理等）。需要将 bee.lua 也编译进来，或者替换掉对 bee.lua 的依赖。

### 8.3 fmt 库依赖
`lua_loader.cpp` 使用了 `fmt::format`。需要引入 fmt 库或替换为 `snprintf`。

### 8.4 ydbase 依赖
yd_core 和 lua_engine 都依赖 ydbase 的 hook 框架和工具类。ydbase 需要作为基础库一起编译。

### 8.5 编译环境
YDWE 使用 VS2019（PlatformToolset v142）。需要确认是否有 VS2019 或更高版本。

## 9. 下一步

1. 确认编译环境（VS2019+）
2. 提取 ydbase 源码并编译
3. 提取 yd_core 源码，改编 nf_register.cpp
4. 提取 lua_engine 源码，改编 lua_loader.cpp
5. 编译整个 DLL 链
6. 通过 callback 加载测试
7. 验证 Lua 引擎和 native 注册是否正常工作
