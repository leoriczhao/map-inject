# japi.dll — 单 DLL 替代方案架构设计

## 1. 目标

用一个 `japi.dll`（Lua 5.3 静态链接）替代 YDWE 的 5 DLL 链（lua53→ydbase→ydwar3→yd_lua_engine→yd_jass_api），在 callback 模式下实现地图自加载 Lua 引擎。

## 2. 整体架构

```
demo.w3x
├── war3map.j          → main() 调用 StartCampaignAI("callback")
├── callback           → MemHackAPI exploit + DLL 加载
├── japi.tga           → japi.dll (单 DLL，含 Lua 5.3)
├── ujapi.tga          → UjAPI.dll (可选，独立工程，非内置 JAPI 的一部分)
└── war3map.lua        → 用户 Lua 脚本
```

### 2.1 执行流程（对齐内置 JAPI）

```
WC3 加载地图
  → war3map.j 编译 → main() 执行
      StartCampaignAI(Player(12), "callback")
  → AI 编译器加载 callback（跳过类型检查）
  → callback::main():
      1. 版本检测（27a/26/24b/24e）
      2. UnlockMemory → 获取任意内存读写
      3. AllocateExecutableMemory → 分配 RWX 内存
      4. ExportFileFromMpq + LoadLibrary → 加载 japi.dll
      5. GetProcAddress("call")
      6. CallCdecl(call, pExportFromMpq)
  → japi.dll::call(pExportFromMpq):
      1. 内部计算 pJassEnvAddress (game.dll + 0xBE3740)
      2. set_vm_context_from_env() → 建立 JASS VM 上下文
      3. init_native_registry() → 扫描全部 natives + 注册自定义 JASS natives
      4. init_lua_bootstrap() → 创建 Lua VM + jass 模块 + 执行 war3map.lua
```

## 3. 核心模块

### 3.1 jass_vm — JASS VM 接口

**Native 扫描**：通过 game.dll 的 native 注册序列扫描

```
game.dll .text 段中每个 native 的注册代码:
  68 XX XX XX XX    push param_str_ptr
  BA XX XX XX XX    mov edx, name_ptr
  B9 XX XX XX XX    mov ecx, func_addr
  E8 XX XX XX XX    call nfunction_add

从 "Deg2Rad" 字符串引用向前偏移 6 字节定位第一个 NativeRegBlock，
然后顺序遍历所有连续的 block，提取 name / param / address。
```

当前扫描结果：**1167 个 JASS natives**，全部映射到 Lua `jass` 模块。

**VM Context**：通过 callback 传入的 pJassEnvAddress 计算

```
链式解引用: [[[pJassEnvAddress] + 20] + 144] + 4
等价于 callback 中的 GetJassContext(1)
```

**jass::call**：调用 JASS native

```cpp
// 内联汇编：保存 ESP → sub esp 分配参数 → rep movsd 拷贝参数
// → call eax → 恢复 ESP（兼容 cdecl/stdcall/thiscall）
```

### 3.2 native_registry — 自定义 JASS Native 注册

通过 `nfunction_add`（地址 0x78913710）注册自定义 native：

```cpp
// nfunction_add 签名: __fastcall(ecx=func, edx=name, [esp]=param_str)
japi_table_add(TestAdd,  "TestAdd",  "(II)I");
japi_table_add(DebugMsg, "DebugMsg", "(I)I");
japi_table_add(SayHello, "SayHello", "(I)I");
```

### 3.3 lua_bootstrap — Lua 引擎

**jass 模块**：通过 metatable `__index` 实现自动 native 映射

```lua
-- 所有 1167 个 JASS native 都可以直接调用
local player = jass.GetLocalPlayer()
jass.DisplayTimedTextToPlayer(player, 0, 0, 60, "hello")
```

**实现原理**：
1. 创建 `jass` 全局 table
2. 设置 metatable `__index` → 查找 JASS native 表
3. 返回 wrapper closure（持有 native name + info 作为 upvalue）
4. 调用时自动类型转换：
   - Lua string → fake jstring_t
   - Lua number → jreal_t（传指针）
   - Lua integer → jinteger_t（传值）

**fake jstring**：手动构造 jstring_t 结构体

```
from_string 链: [val+8] → inner_ptr → [inner+28] → const char*

构造方法:
  outer[0:8]  = 0
  outer[8:12] = &inner        (指向 inner 的指针)
  inner[0:28] = 0
  inner[28:32] = msg          (指向 C 字符串)
  jstring_t = outer
```

## 4. 与内置 JAPI (wenhao_plugin) 对比

| 维度 | 内置 JAPI (wenhao_plugin) | japi.dll |
|------|----------|----------|
| **DLL 数量** | 1 (wenhao_plugin，UjAPI 是独立工程非其一部分) | 1 (japi.dll，Lua 静态链接) |
| **JASS Native 注册** | nfunction_add（相同） | nfunction_add（相同） |
| **Native 调度** | hashtable + UnitId hook | Lua metatable __index |
| **Native 发现** | 硬编码 50+ natives | 扫描全部 1167 个，自动映射 |
| **Lua→JASS 调用** | jass.common 模块 | jass 模块（直接属性访问） |
| **字符串处理** | 通过 JASS VM 原生 create_string | 手动构造 fake jstring |
| **类型安全** | 需要手动声明签名 | 自动解析 param spec 字符串 |
| **callback 利用** | 相同 MemHackAPI | 相同 MemHackAPI |
| **process 注入** | 不需要 | 不需要 |
| **force_watch** | 不需要 | 不需要 |

**三个参考源**：
1. **内置 JAPI (wenhao_plugin)** — 主要参考，无源码仅逆向。我们的架构和流程方案（单 DLL 加载、`nfunction_add` 注册、`jass.message/common/slk/runtime/japi` 模块结构）均参考它。
2. **UjAPI** — 国外半开源独立工程，参考其扩展 native API 设计（扩展 handle 类型、扩展 native 声明）。
3. **YDWE** — 有源码，参考其 `nfunction_add` 发现机制（`nf_register.cpp`）、runtime native 注册（`libs_japi.cpp`）、hook 机制（`hook.cpp`）等实现细节。

**原理交集**：虽然注入方式不同，但四者进入 game.dll 内部后，都通过 `nfunction_add` 注册自定义 native，都直接调用 game.dll 内部函数。japi.dll 的优势在于更简洁（单 DLL）、更自动化（自动扫描全部 natives）、更低耦合（不依赖特定 WC3 版本的硬编码偏移）。

## 5. 相关文档

- 开发路线图与进度：`docs/planning/ROADMAP.md`
- 关键地址表：`docs/architecture/ADDRESSES.md`
- 参考源详情：`docs/reference/REFERENCES.md`
