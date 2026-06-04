# 参考源

## 主要参考：内置 JAPI（wenhao_plugin）

架构和流程方案的主要参考。无源码，仅有逆向分析。

- 逆向报告：`reference/builtin-japi/wenhao_plugin逆向分析报告.html`
- 原始地图文件：`reference/builtin-japi/original.w3x`
- callback 代码（JASS）：`reference/builtin-japi/callback`
- Lua 模块结构参考：`reference/builtin-japi/main.lua`

内置 JAPI 的 Lua 模块设计（我们照此实现）：

```
jass.message   — 键盘/鼠标/命令拦截
jass.common    — JASS native 调用
jass.slk       — SLK 数据读取
jass.runtime   — 运行时 native 注册
jass.japi      — 扩展 JAPI natives
```

关键区分：内置 JAPI 不包含 UjAPI。UjAPI 是另一个独立工程。

## 次要参考：UjAPI（国外半开源，运行时进程注入）

- `reference/ujapi/UjAPI.j` — 扩展 handle 类型声明
- `reference/ujapi/ujapi_common.j` — 扩展 native 声明（common.j 风格）
- `reference/ujapi/UjAPI_Native参考手册.html` — Native 参考文档
- `reference/ujapi/_ujapi_cache/` — 完整地图文件（包含 UjAPI.dll, lua53.dll, main.lua 等）

UjAPI 提供扩展的 handle 类型（projectile, framehandle, sprite 等）和大量扩展 natives。参考其 native API 设计，但不照搬其多 DLL 架构。**注意：UjAPI 通过运行时进程注入加载，不是 callback exploit。**

## 次要参考：YDWE（有源码，运行时进程注入）

- Native 注册机制：`../Development/Core/ydwar3/warcraft3/jass/nf_register.cpp`
- JAPI 模块：`../Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/libs_japi.cpp`
- Hook 机制：`../Development/Core/ydwar3/warcraft3/jass/hook.cpp`
- JASS 调用：`../Development/Plugin/Warcraft3/yd_lua_engine/lua_engine/lua_to_nativefunction.cpp`

YDWE 通过外部启动器注入 DLL 链到 War3 进程。其 `nfunction_add` 发现机制（Hijack storm_alloc + TlsGetValue）比我们复杂。我们采用更简单的方法：直接在 `.text` 段扫描 20 字节注册块（见 `jass_vm.cpp::scan_natives`）。
