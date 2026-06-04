# JASS → Lua 桥接设计（内置 JAPI 路线）

## 一、核心思路

**一张地图搞定一切。** DLL 劫持一个已有的 JASS 原生函数（`UnitId`），所有 JASS→Lua 调用走统一的哈希表传参 + 字符串名分发。war3map.j 里不需要写 `native` 声明，所有桥接函数都是普通的 JASS 函数，pjass 完全兼容。

## 二、和 YDWE 路线的区别

| | YDWE trampoline | 内置 JAPI（我们做这个） | 我们现在的 japi.japi |
|---|---|---|---|
| JASS 侧写法 | `native` 声明 | 普通 JASS 函数 | `native` 声明 |
| 参数传递 | JASS VM 栈 | 哈希表 | JASS VM 栈 |
| DLL 入口 | nfunction_add | 劫持 UnitId | nfunction_add |
| pjass 兼容 | 需预处理 | **零改动** | 需预处理 |
| 地图自包含 | 是 | **是** | 是 |

现在的 `japi.japi` 用的是 `nfunction_add` 注册新原生，JASS 侧需要 `native` 声明。这条路技术上可行但**不是内置 JAPI 的做法**——用户必须手动写 `native` 或者靠脚本生成。

内置 JAPI 的做法是：**JASS 侧全是普通函数，不需要任何 native 声明。**

## 三、完整调用流程

### 3.1 JASS → Lua 调用

```
JASS 函数 (war3map.j)        DLL (japi.dll)              Lua (war3map.lua)
══════════════════════        ════════════                 ════════════════

call EXExecuteScript("hi")
  │
  ├─ SaveStr(ht, key, 0, "(S)S")    // 写入参数格式
  ├─ SaveStr(ht, key, 1, "hi")      // 写入参数
  │
  ├─ UnitId("EXExecuteScript") ────→ UnitId 被劫持
  │                                   │
  │                              从 ht 读出 spec "(S)S"
  │                              从 ht 读出 params
  │                              查找 "EXExecuteScript" handler
  │                                   │
  │                              lua_pcall(handler, params) → "HELLO"
  │                                   │
  │                              结果写回 ht:
  │                              SaveStr(ht, key, 0, "HELLO")
  │                                   │
  │← 返回 0 ─────────────────────────┘
  │
  └─ result = LoadStr(ht, key, 0)   // 读出 "HELLO"
  return result
```

### 3.2 Lua → JASS 调用（不变，已实现）

```
Lua: jass.CreateTimer()  →  jass.common __index
                           →  jass_call_native_function
                           →  JASS VM 直接执行原生函数
                           →  返回值转 Lua 类型
```

## 四、DLL 需要改的

### 4.1 劫持 UnitId（新的拦截点）

现在 DLL 已经劫持了 `AbilityId` 用于 `exec-lua:` 引导。需要新增劫持 `UnitId` 用于通用 JASS→Lua 分发。

```
UnitId_Hook(jstring_t name):
    if name = "EXExecuteScript" or 其他注册名:
        1. 读取哈希表 (ht, key, 0) → 获取 spec 字符串
        2. 解析 spec 获取参数数量 n
        3. 从哈希表 (ht, key, 1..n) 读取 n 个参数
        4. 查找 handler → lua_pcall
        5. 返回值写回哈希表 (ht, key, 0)
        return 0
    else if name 是纯数字字符串 (如 "1048576"):
        初始化阶段——这是传递哈希表句柄
        记录 ht_handle = parseInt(name)
        return UnitId_orig(name)
    else:
        return UnitId_orig(name)
```

### 4.2 读取/写入哈希表

DLL 需要能操作游戏内的哈希表。哈希表是 JASS 原生对象，通过调用以下 JASS 原生实现：

```
// 这些是 game.dll 里的原生函数，DLL 可以直接调用
SaveInteger(ht, key, slot, value)
SaveReal(ht, key, slot, value)
SaveStr(ht, key, slot, value)
SaveBoolean(ht, key, slot, value)
LoadInteger(ht, key, slot)
LoadReal(ht, key, slot)
LoadStr(ht, key, slot)
LoadBoolean(ht, key, slot)
```

DLL 已经能通过 `jass::vm::find_native("SaveInteger")` 拿到函数地址，直接 `jass::call()` 调用。

### 4.3 哈希表句柄传递机制

war3map.j 初始化时把哈希表句柄传给 DLL：

```jass
// war3map.j — initializePlugin()
function initializePlugin takes nothing returns integer
    call StartCampaignAI(Player(12), "callback")     // → DLL 加载
    call UnitId(I2S(GetHandleId(japi_ht)))            // → DLL 记录哈希表句柄
    return 0
endfunction
```

DLL 在 `UnitId_Hook` 中识别这个调用：
- 参数是纯数字字符串 → 这是初始化，记录哈希表句柄
- 参数是函数名 → 这是业务调用，走分发

## 五、war3map.j 模板（桥接函数）

### 5.1 基础框架

```jass
globals
    // === japi 桥接基础设施 ===
    hashtable japi_ht = InitHashtable()
    integer japi__key = StringHash("jass")
endglobals

// === DLL 加载 ===
function japi_init takes nothing returns integer
    call ExecuteFunc("japi_init_memory")
    call StartCampaignAI(Player(PLAYER_NEUTRAL_AGGRESSIVE), "callback")
    // 通知 DLL 哈希表句柄
    call UnitId(I2S(GetHandleId(japi_ht)))
    return 0
endfunction
```

### 5.2 桥接函数模板

每种参数组合一个模板：

```jass
// ()S — 无参数，返回字符串
function japi_call_0_S takes string name returns string
    call SaveStr(japi_ht, japi__key, 0, "()S")
    call UnitId(name)
    return LoadStr(japi_ht, japi__key, 0)
endfunction

// (I)I — 一个整数参数，返回整数
function japi_call_1I_I takes string name, integer a1 returns integer
    call SaveStr(japi_ht, japi__key, 0, "(I)I")
    call SaveInteger(japi_ht, japi__key, 1, a1)
    call UnitId(name)
    return LoadInteger(japi_ht, japi__key, 0)
endfunction

// (II)I — 两个整数参数，返回整数
function japi_call_2I_I takes string name, integer a1, integer a2 returns integer
    call SaveStr(japi_ht, japi__key, 0, "(II)I")
    call SaveInteger(japi_ht, japi__key, 1, a1)
    call SaveInteger(japi_ht, japi__key, 2, a2)
    call UnitId(name)
    return LoadInteger(japi_ht, japi__key, 0)
endfunction

// (S)S — 字符串参数，返回字符串
function japi_call_1S_S takes string name, string a1 returns string
    call SaveStr(japi_ht, japi__key, 0, "(S)S")
    call SaveStr(japi_ht, japi__key, 1, a1)
    call UnitId(name)
    return LoadStr(japi_ht, japi__key, 0)
endfunction

// (R)V — 实数参数，无返回值
function japi_call_1R_V takes string name, real a1 returns nothing
    call SaveStr(japi_ht, japi__key, 0, "(R)V")
    call SaveReal(japi_ht, japi__key, 1, a1)
    call UnitId(name)
endfunction

// ... 覆盖常用组合，总共约 15-20 个模板
```

### 5.3 用户级封装（有语义的函数名）

```jass
// 用户友好的封装——调用底层模板
function EXExecuteScript takes string str returns string
    return japi_call_1S_S("EXExecuteScript", str)
endfunction

function GetMouseX takes nothing returns real
    call SaveStr(japi_ht, japi__key, 0, "()R")
    call UnitId("GetMouseX")
    return LoadReal(japi_ht, japi__key, 0)
endfunction
```

## 六、war3map.lua 侧

```lua
local japi = require "jass.japi"

-- 注册 JASS→Lua handler
japi("EXExecuteScript", "(S)S", function(str)
    -- str 来自 JASS
    local ok, result = load("return (" .. str .. ")")
    if ok then
        return tostring(result())
    end
    return ""
end)

japi("GetMouseX", "()R", function()
    -- ... 获取鼠标 X 坐标 ...
    return 1234.5
end)

-- JASS 调用 EXExecuteScript("1+1") → Lua 执行 → 返回 "2"
```

## 七、需要的改动清单

### DLL 改动（`native_registry.cpp`）

1. ✅ `AbilityId_Hook` — 已有，处理 `exec-lua:N` 引导
2. 🔨 **新增 `UnitId_Hook`** — 处理通用 japi 分发
   - 识别初始化调用（纯数字 → 记录哈希表句柄）
   - 识别业务调用（函数名 → 读哈希表 → 分发到 Lua）
3. 🔨 **哈希表读写封装** — 封装 `SaveXXX`/`LoadXXX` 调用
4. 🗑️ 移除 `nfunction_add` 注册自定义 native 的逻辑（不再需要）

### war3map.j 改动

1. 🔨 新增约 15 个 `japi_call_*` 模板函数
2. 🔨 新增 `japi_init()` → DLL 加载 + 哈希表句柄传递
3. 🔨 新增用户级封装函数（按需）

### war3map.lua 改动

1. 🔨 `japi` 模块改为 UnitId 拦截图模式（不再走 nfunction_add）

## 八、哈希表槽位约定

| 槽位 | 用途 | 类型 |
|------|------|------|
| 0 | spec 字符串（调用方写）/ 返回值（DLL 写） | string |
| 1 | 第 1 个参数 | 按 spec |
| 2 | 第 2 个参数 | 按 spec |
| ... | ... | ... |

## 九、spec 格式

沿用现有格式，和内置 JAPI 一致：

| Spec | 含义 |
|------|------|
| `()V` | 无参数，无返回值 |
| `()I` | 无参数，返回整数 |
| `()R` | 无参数，返回实数 |
| `(I)I` | 1 整数参数，返回整数 |
| `(II)I` | 2 整数参数，返回整数 |
| `(S)S` | 1 字符串参数，返回字符串 |
| `(R)V` | 1 实数参数，无返回值 |
| `(IR)V` | 整数+实数参数，无返回值 |
| `(IHplayer;)I` | 整数+玩家 handle 参数，返回整数 |

## 十、为什么必须是这个方案

1. **一张地图**：war3map.j + war3map.lua + callback + japi.dll(tga) 全部在 MPQ 里，不需要外部工具
2. **零 native 声明**：所有桥接函数是普通 JASS 函数，pjass 不报错
3. **内置 JAPI 验证过的**：这个方案在成千上万张地图上跑过
4. **免预处理**：不需要 Python 脚本、不需要 JASS 编译器插件
