# AGENT.md — 工作流 & 开发约定

## 概览

本项目有三条工作流，互相独立但可串联：

| 工作流 | 输入 | 输出 | 脚本 |
|--------|------|------|------|
| **DLL 编译** | `src/src/**/*.cpp` | `src/build/bin/Release/yd_japi.dll` | `src/build.bat` |
| **地图打包** | `map/*.lua` + 基座地图 + DLL | `build/output.w3x` | `tools/pack.ps1` |
| **部署测试** | `build/output.w3x` | War3 日志 | `tools/deploy.ps1` |

## 架构

```
war3map.j（干净，只调标准 JASS native）
  └→ initializePlugin()
       └→ StartCampaignAI(Player(12), "callback")  ← 加载 exploit
            └→ callback（内存 exploit + DLL 加载）
                 └→ japi.tga (yd_japi.dll)
                      ├→ hook UnitId → 分发到 Lua handler
                      └→ 创建 Lua VM → 加载 war3map.lua
```

- **war3map.j**：干净的 JASS 代码，只调 `UnitId`/`SaveStr`/`LoadReal` 等标准 native
- **callback**：exploit 代码，通过 `StartCampaignAI` 加载（AI 脚本不走 JASS 检测）
- **DLL**：hook `UnitId` 做 JASS→Lua 分发，哈希表传参

---

## 工作流 1：DLL 开发（C++ 侧）

修改 C++ 源码 → 编译 → 产出 DLL。

```powershell
cd src
.\build.bat
# 输出：src/build/bin/Release/yd_japi.dll
```

**目录约定**：

| 路径 | 用途 |
|------|------|
| `src/src/yd_core/` | YDWE 核心（改编自 ydwar3） |
| `src/src/lua_engine/` | Lua 引擎 + bridge dispatch |
| `src/src/extensions/` | 自定义扩展 native |
| `src/vendor/lua-5.4/` | Lua 5.4 源码（静态编译） |

**添加自定义 native**：

1. 编辑 `src/src/extensions/custom_natives.cpp`
2. 用 `jass::japi_add()` 注册函数
3. 重新编译

---

## 工作流 2：地图开发（Lua 侧）

编写 Lua 脚本 → 打包进地图。

```powershell
.\tools\pack.ps1
# 输出：build/output.w3x
```

**目录约定**：

| 路径 | 用途 |
|------|------|
| `map/war3map.lua` | 用户主入口（必须存在） |
| `map/lib/*.lua` | 用户库模块，`require "lib.xxx"` |
| `map/tests/*.lua` | 测试模块，`require "tests.xxx"` |
| `build/output.w3x` | 打包输出（不要提交） |

**pack.ps1 流程**（不动 war3map.j）：

1. 解包基座地图 → `build/work/`
2. 合并 `map/` 下 Lua → `build/work/war3map.lua`
3. 复制 DLL → `build/work/japi.tga`
4. 复制 callback → `build/work/callback`
5. 重新打包为 `build/output.w3x`

**Lua 侧注册 JAPI 函数**：

```lua
local japi = require "jass.japi"

-- 注册一个 JASS 可调用的函数
japi("TestAdd", "(II)I", function(a, b)
    return a + b
end)

-- JASS 侧通过 UnitId("TestAdd") 调用
```

---

## 工作流 3：部署测试

```powershell
.\tools\deploy.ps1
# 打包 + 部署到 War3 + 启动 + 等待测试结果
```

---

## 完整开发循环

### 场景 A：只改 Lua（最常见）

```powershell
# 1. 编辑 map/war3map.lua
# 2. 打包 + 测试
.\tools\deploy.ps1
```

### 场景 B：改 C++ DLL

```powershell
# 1. 编辑 src/src/...
# 2. 编译 DLL
cd src; .\build.bat; cd ..
# 3. 打包 + 测试
.\tools\deploy.ps1
```

---

## 配置管理

War3 路径通过以下方式设置（优先级从高到低）：

1. **命令行参数**：`-War3Dir "C:\War3"`
2. **环境变量**：`$env:WAR3_DIR = "C:\War3"`
3. **本地配置文件**：`tools/config.local.ps1`（gitignored）
4. **默认值**：`D:\Warcraft III Frozen Throne 1.27a publish`

**首次设置**：

```powershell
Copy-Item tools\config.example.ps1 tools\config.local.ps1
# 编辑 config.local.ps1
```

---

## 脚本参数速查

**pack.ps1**：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `-BaseMap` | `reference\base.w3m` | 基座地图 |
| `-DllPath` | `src\build\bin\Release\yd_japi.dll` | DLL 路径 |
| `-OutFile` | `build\output.w3x` | 输出路径 |
| `-MapDir` | `map` | Lua 源码目录 |

**deploy.ps1**：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `-BaseMap` | `reference\base.w3m` | 基座地图 |
| `-DllPath` | 同 pack.ps1 | DLL 路径 |
| `-War3Dir` | 配置/默认 | War3 安装目录 |
| `-Timeout` | 120 | 超时秒数 |
| `-NoKill` | 开关 | 不杀 War3 |
