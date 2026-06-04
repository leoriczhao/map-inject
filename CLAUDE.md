# map-inject — WAR3 地图注入方案（YDWE 基座 + 扩展）

## 项目目标

基于 YDWE 核心源码编译适配的 DLL 链作为基座，通过 callback exploit 在 MPQ 模式下加载，提供 Lua 引擎 + JASS 扩展 native。在此基础上编写自定义扩展。

## 技术路线

**Callback exploit（入口）→ YDWE DLL 链（基座，自编译适配）→ 自定义扩展 DLL**

- YDWE 的 `nf_register.cpp` hook `StormAlloc` 检测 VM 初始化，在 callback 模式下不触发（VM 已初始化）
- 需要改编：改为 DLL 加载时直接调用 `nfunction_add` 注册 native
- YDWE 的 `lua_loader.cpp` hook war3map.j 加载，在 callback 模式下需要适配
- 需要改编：改为 `Initialize()` 里主动创建 lua_State 并加载 war3map.lua

## 核心原理

1. **Callback exploit**：利用 jasshelper 类型混淆（`l__` 前缀）获取任意内存读写能力
2. **DLL 加载**：ExportFileFromMpq + LoadLibrary 加载 YDWE DLL 链
3. **Native 注册**：改编 nf_register.cpp，在 Initialize() 时直接调 nfunction_add
4. **Lua 引擎**：改编 lua_loader.cpp，Initialize() 里主动创建 lua_State + 加载 war3map.lua

## 项目结构

```
map-inject/
├── CLAUDE.md
├── AGENT.md
│
├── map/                         ← 地图开发目录
│   ├── demo.w3x                # 测试地图
│   ├── war3map.lua             # 用户 Lua 脚本
│   └── lib/                    # Lua 工具库
│
├── src/                         ← DLL 源码目录
│   ├── CMakeLists.txt
│   ├── build.bat
│   ├── src/
│   │   ├── dllmain.cpp
│   │   ├── yd_core/            ← 从 YDWE 提取的核心（改编）
│   │   │   ├── hashtable.h     # VM 内存布局定义
│   │   │   ├── jass.h/cpp      # JASS 类型、字符串、调用
│   │   │   ├── hook.h/cpp      # native 注册/hook 框架
│   │   │   ├── nf_register.cpp # 改编：直接注册而非 hook StormAlloc
│   │   │   ├── func_value.h/cpp
│   │   │   └── war3_searcher.h/cpp
│   │   ├── lua_engine/         ← 从 YDWE 提取的 Lua 引擎（改编）
│   │   │   ├── lua_loader.cpp  # 改编：Initialize() 主动启动
│   │   │   ├── open_lua_engine.cpp
│   │   │   ├── jassbind.h/cpp
│   │   │   ├── callback.h/cpp
│   │   │   └── libs_*.cpp      # jass.common, jass.hook, jass.japi 等
│   │   └── extensions/         ← 自定义扩展 native
│   └── vendor/
│       └── lua-5.3.6-src/
│
├── tools/                       ← 构建 & 打包工具
│   ├── callback                # exploit JASS 代码
│   ├── core/                   # MemHackAPI JASS 库
│   ├── pack.ps1                # MPQ 打包脚本（Lua 合并 + DLL 注入 + patch）
│   ├── deploy.ps1              # 打包 + 部署 + 启动 War3 + 等结果
│   ├── pjass.exe               # JASS 语法检查
│   └── w3x2lni/                # 地图格式转换
│
├── docs/                        ← 文档
│   ├── architecture/
│   ├── planning/
│   └── analysis/
│
└── reference/                   ← 逆向参考
    ├── builtin-japi/            # 内置 JAPI（callback exploit 来源）
    ├── ujapi/                   # UjAPI（2333 扩展 native 参考）
    ├── ujapi-full/              # UjAPI GitHub 完整仓库
    ├── YDWE/                    # YDWE 完整源码（编译基座）
    └── old-japi/                # 旧的单 DLL 方案（存档参考）
```

## YDWE 核心源码位置（需提取改编）

| 模块 | 源码路径 | 用途 |
|------|----------|------|
| ydbase | `Development/Core/ydbase/` | hook 框架（IAT/EAT/inline） |
| ydwar3 | `Development/Core/ydwar3/` | JASS VM 接口、native 注册、内存结构 |
| yd_lua_engine | `Development/Plugin/Warcraft3/yd_lua_engine/` | Lua 引擎、jass.* 模块 |
| yd_jass_api | `Development/Plugin/Warcraft3/yd_jass_api/` | EX* 扩展 native |

## Callback 模式适配要点

| YDWE 原始机制 | 问题 | 改编方案 |
|---|---|---|
| nf_register.cpp hook StormAlloc | VM 已初始化，hook 不触发 | Initialize() 里直接调 nfunction_add |
| lua_loader.cpp hook war3map.j | war3map.j 已执行 | Initialize() 里主动创建 lua_State |
| yd_jass_api 依赖 event_add 信号 | 信号不会触发 | 直接调 japi_add/japi_table_add |

## 构建（DLL 编译）

```powershell
cd src
.\build.bat
# 输出：src/build/bin/Release/yd_japi.dll
```

## 打包 & 测试（地图开发）

```powershell
# 打包地图（合并 Lua + 注入 DLL + patch war3map.j）
.\tools\pack.ps1
# 输出：build/output.w3x

# 打包 + 部署到 War3 + 启动 + 等待测试结果
.\tools\deploy.ps1

# 详细工作流参见 AGENT.md
```
