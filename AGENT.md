# AGENT.md — 工作流 & 开发约定

## 两个隔离的工作流

本项目有两条独立的工作流，互不耦合。

---

### 工作流 1：地图开发（用户侧）

用户编写 Lua 脚本，打包进地图，部署到 War3 测试。

```
编辑 map/war3map.lua, map/lib/*.lua, map/tests/*.lua
        ↓
  tools/pack.ps1
        ↓
  build/output.w3x → 部署到 War3 → 启动测试
```

**目录约定**：

| 路径 | 用途 |
|------|------|
| `lalala.w3m` | 基座地图（固定不变，打包的输入） |
| `map/war3map.lua` | 用户主入口（必须存在） |
| `map/lib/*.lua` | 用户库模块，通过 `require "lib.xxx"` 引用 |
| `map/tests/*.lua` | 用户测试模块，通过 `require "tests.xxx"` 引用 |
| `build/output.w3x` | 打包输出（自动生成，不要提交） |

**脚本**：

```powershell
# 打包地图（使用默认路径）
.\tools\pack.ps1

# 打包地图（指定 DLL 路径）
.\tools\pack.ps1 -DllPath "path\to\yd_japi.dll"

# 打包 + 部署 + 启动 War3 + 等待测试结果
.\tools\deploy.ps1

# 打包 + 部署 + 保持 War3 运行
.\tools\deploy.ps1 -NoKill
```

**pack.ps1 参数**：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `-DllPath` | `src\build\bin\Release\yd_japi.dll` | DLL 路径 |
| `-BaseMap` | `lalala.w3m` | 基座地图 |
| `-OutFile` | `build\output.w3x` | 输出路径 |
| `-MapDir` | `map` | Lua 源码目录 |

**deploy.ps1 参数**：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `-DllPath` | 同 pack.ps1 | DLL 路径（透传） |
| `-LogFile` | `D:\...\japi-debug.log` | War3 日志路径 |
| `-Timeout` | 120 | 等待超时（秒） |
| `-NoKill` | 开关 | 测试后不杀 War3 |

**pack.ps1 内部流程**：

1. 解包 `lalala.w3m` → `build/work/`
2. 合并 `map/` 下所有 Lua 文件 → `build/work/war3map.lua`（自动内联 require）
3. 用 `patch_war3map_j.py` 给 war3map.j 注入 exploit + bridge
4. 复制 DLL 为 `japi.tga` + 复制 callback 文件
5. 重新打包为 `build/output.w3x`

---

### 工作流 2：DLL 开发（C++ 侧）

开发者修改 C++ 源码，编译 DLL。

```
编辑 src/src/yd_core/*.cpp, src/src/lua_engine/*.cpp, src/src/extensions/*.cpp
        ↓
  src/build.bat
        ↓
  src/build/bin/Release/yd_japi.dll
```

**目录约定**：

| 路径 | 用途 |
|------|------|
| `src/src/yd_core/` | YDWE 核心（改编自 ydwar3） |
| `src/src/lua_engine/` | Lua 引擎（改编自 yd_lua_engine） |
| `src/src/extensions/` | 自定义扩展 native |
| `src/vendor/lua-5.4/` | Lua 5.4 源码（静态编译） |
| `src/build.bat` | 编译脚本（调用 CMake + VS2022） |
| `src/build/bin/Release/yd_japi.dll` | 编译输出 |

**编译**：

```powershell
cd src
.\build.bat
# 输出：src/build/bin/Release/yd_japi.dll
```

编译完成后，`tools/pack.ps1` 和 `tools/deploy.ps1` 会自动从默认路径读取 DLL。

---

## 工作流隔离原则

- **地图开发不需要编译 DLL**：直接用已编译好的 DLL 打包
- **DLL 开编译不需要打包**：编译完就行，打包是地图开发的事
- **两条工作流通过 DLL 路径耦合**：默认路径 `src/build/bin/Release/yd_japi.dll`，可通过 `-DllPath` 覆盖
- **`tools/test.ps1` 已删除**：被 `tools/deploy.ps1` 替代
