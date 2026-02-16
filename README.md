# CJAPP 工程说明

本文档说明两个重点：
- `.vscode` 目录下各配置文件的作用、协作关系和使用规范。
- 根目录 `CmakeLists.txt`（当前工程实际配置）的详细解释。

## 一、目录约定

当前工程的关键结构（与构建直接相关）如下：

```text
.
├─ main.cpp
├─ CmakeLists.txt
├─ ModuleA/
│  ├─ include/ModuleA.hpp
│  └─ src/ModuleA.cpp
├─ ModuleB/
│  ├─ include/ModuleB.hpp
│  └─ src/ModuleB.cpp
├─ .vscode/
│  ├─ tasks.json
│  ├─ launch.json
│  ├─ settings.json
│  └─ c_cpp_properties.json
└─ out/   # 构建产物输出目录
```

## 二、.vscode 使用规范与详细解释

`.vscode` 是 VS Code 的工作区级配置，推荐只提交与团队协作相关、可复用的设置。

### 1. `tasks.json`：构建任务定义

用途：定义“执行什么命令”。

当前任务分为两类：
- 初始化构建（`Init+Build`）：先 `configure` 再 `build`。
- 增量构建（`Build`）：只执行 `build`，用于日常快速编译。

当前文件中的关键任务：
- `CMake Configure (MSVC)`：配置到 `build-msvc`。
- `CMake Build (Debug-MSVC)`：构建 `build-msvc`。
- `CMake Init+Build (MSVC)`：顺序执行上面两个。
- `CMake Configure (GDB)`：配置到 `build-gdb`（MinGW + g++）。
- `CMake Build (GDB)`：构建 `build-gdb`。
- `CMake Init+Build (GDB)`：顺序执行上面两个。

使用规范：
- 首次打开工程、修改 `CmakeLists.txt`、切换编译选项时，用 `Init+Build`。
- 平时改代码调试，用 `Build`（增量编译，速度快）。
- MSVC 与 GDB 分离到不同构建目录，避免生成器冲突（非常重要）。

### 2. `launch.json`：调试启动配置

用途：定义“如何启动调试器”。

当前有 4 套配置：
- `Debug (MSVC, init once)`
- `Debug (MSVC, incremental)`
- `Debug (gdb, init once)`
- `Debug (gdb, incremental)`

关键字段说明：
- `program`：要调试的可执行文件（当前为 `${workspaceFolder}/out/CJAPP.exe`）。
- `type`：调试器类型（`cppvsdbg` 为 MSVC；`cppdbg` + `MIMode=gdb` 为 GDB）。
- `preLaunchTask`：调试前执行哪个构建任务。
- `miDebuggerPath`：GDB 路径。

使用规范：
- 首次或改构建配置：选择 `init once`。
- 日常调试：选择 `incremental`。

### 3. `settings.json`：工作区行为设置

用途：定义 VS Code 在当前工程中的默认行为。

当前关键设置：
- `cmake.buildDirectory`：默认构建目录 `build-gdb`。
- `cmake.configureOnOpen`：打开工程时自动配置。
- `C_Cpp.default.compileCommands`：指向 `build-gdb/compile_commands.json`，让 IntelliSense 使用真实编译参数。

使用规范：
- 若切换主力工具链（如从 g++ 切到 clang++），同步更新该路径和工具链配置。
- 不要让多个工具链共享同一个 build 目录。

### 4. `c_cpp_properties.json`：C/C++ 智能感知配置

用途：配置编辑器层面的语义分析（补全、跳转、红线），不直接控制真实构建。

当前关键项：
- `compilerPath`：`D:/mingw64/bin/g++.exe`
- `intelliSenseMode`：`windows-gcc-x64`
- `cppStandard`：`c++17`
- `compileCommands`：`${workspaceFolder}/build-gdb/compile_commands.json`

使用规范：
- 优先依赖 `compile_commands.json`，保证 IntelliSense 与真实编译一致。
- 修改构建目录时，记得同步更新 `compileCommands` 路径。

### 5. `.vscode` 文件协作关系

整体流程：
1. `launch.json` 触发 `preLaunchTask`。
2. `tasks.json` 调用 `cmake configure/build`。
3. CMake 读取 `CmakeLists.txt` 生成并构建。
4. 可执行文件输出到 `out/`。
5. 调试器加载 `out/CJAPP.exe`。
6. IntelliSense 通过 `settings.json` + `c_cpp_properties.json` 读取 `compile_commands.json`。

## 三、CmakeLists.txt 详细解释（当前版本）

以下按逻辑块解释当前 `CmakeLists.txt`：

### 1. CMake 与项目声明

- `cmake_minimum_required(VERSION 3.16)`：要求最低 CMake 版本。
- `project(CJAPP LANGUAGES CXX)`：项目名 `CJAPP`，语言为 C++。

### 2. 默认构建类型

- 若未指定构建类型且非多配置生成器，默认设置为 `Debug`。
- 作用：方便本地调试。

### 3. C++ 标准设置

- `CMAKE_CXX_STANDARD 17`
- `CMAKE_CXX_STANDARD_REQUIRED ON`
- `CMAKE_CXX_EXTENSIONS OFF`

含义：使用标准 C++17，避免编译器私有扩展。

### 4. 输出目录统一

- `OUTPUT_DIR` 设置为 `${PROJECT_SOURCE_DIR}/out`。
- `CMAKE_RUNTIME_OUTPUT_DIRECTORY`：可执行文件输出目录。
- `CMAKE_LIBRARY_OUTPUT_DIRECTORY`：动态库输出目录。
- `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`：静态库/导入库输出目录。
- `foreach(cfg ...)`：对 `Debug/Release/...` 等配置统一输出到 `out`。

### 5. 编译告警

- MSVC：`/W4 /permissive-`
- GCC/Clang：`-Wall -Wextra -Wpedantic`

作用：提高代码质量，尽早发现潜在问题。

### 6. 两个关键开关（ON/OFF）

- `USE_LIB_MODE`
  - `OFF`：多源文件直接编译进可执行文件。
  - `ON`：先编 `ModuleA/ModuleB` 库，再链接主程序。
- `BUILD_SHARED`
  - `OFF`：库类型为静态库（`STATIC`）。
  - `ON`：库类型为动态库（`SHARED`）。

代码中先根据 `BUILD_SHARED` 计算 `LIB_TYPE`，再在 `USE_LIB_MODE=ON` 时应用。

### 7. 两种构建策略

#### 策略 A：`USE_LIB_MODE=OFF`

执行：
- `add_executable(CJAPP main.cpp ModuleA/src/ModuleA.cpp ModuleB/src/ModuleB.cpp)`
- 给 `CJAPP` 添加 `ModuleA/include` 和 `ModuleB/include` 头文件路径。

特点：
- 配置简单。
- 适合小项目或初学阶段。

#### 策略 B：`USE_LIB_MODE=ON`

执行：
- `add_library(ModuleA ${LIB_TYPE} ...)`
- `add_library(ModuleB ${LIB_TYPE} ...)`
- `add_executable(CJAPP main.cpp)`
- `target_link_libraries(CJAPP PRIVATE ModuleA ModuleB)`

特点：
- 结构清晰，利于模块复用。
- 可切换静态/动态库。

### 8. 配置日志输出

通过 `message(STATUS ...)` 输出：
- 项目名
- 构建类型
- 构建模式（库模式开关）
- 库类型（STATIC/SHARED）
- 编译器路径
- 输出目录

作用：在配置阶段快速确认当前生效参数。

## 四、常用构建命令（建议）

### 1. GDB 模式 - 初始化一次（推荐首次）

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=OFF -DBUILD_SHARED=OFF
cmake --build build-gdb -j
```

### 2. GDB 模式 - 日常增量编译

```powershell
cmake --build build-gdb -j
```

### 3. 切换为“先编库再链接”

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=ON -DBUILD_SHARED=OFF
cmake --build build-gdb -j
```

### 4. 切换为“动态库”

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=ON -DBUILD_SHARED=ON
cmake --build build-gdb -j
```

## 五、建议的团队规范

- `.vscode/tasks.json`、`.vscode/launch.json`、`.vscode/settings.json` 建议纳入版本管理。
- 用户个人偏好（字体、主题、快捷键）不要写入项目。
- 构建目录（`build-*`）和产物目录（`out/`）建议在 `.gitignore` 中忽略。
- 修改 `CmakeLists.txt` 后，优先执行一次 `Init+Build`，再切回增量构建。

---

若后续要支持多平台（Linux/macOS）或多目标（测试、安装、打包），建议在现有 `CmakeLists.txt` 基础上继续拆分为 `cmake/modules/*.cmake`，把“选项定义、目标创建、安装规则”分层管理。

## 六、常见问题排查手册

### 1. 报错：`Could not create named generator MinGW`

原因：
- 命令参数被错误拆分，`MinGW Makefiles` 没被当作一个完整参数。

排查与修复：
- 在 `tasks.json` 中使用 `command + args` 数组写法，不要把整条命令写成一个字符串。
- 确认 `-G` 后面的参数是 `"MinGW Makefiles"`。

### 2. 报错：MSVC 和 MinGW 来回切换后，构建目录异常

典型现象：
- `CMakeCache.txt directory is different`。
- `source ... does not match source ...`。

原因：
- 不同生成器复用了同一个 build 目录。

修复：
- 始终分离构建目录：
  - MSVC：`build-msvc`
  - GDB/MinGW：`build-gdb`
- 已在当前 `tasks.json` 中按此规范配置。

### 3. 每次按 F5 都像“全量重编”

原因：
- 调试前每次都执行了 `configure`，导致重新生成构建系统。

修复：
- 日常使用 `incremental` 调试项（只 build，不 configure）。
- 只有在修改 `CmakeLists.txt` 或切换选项后，才使用 `init once`。

### 4. VS Code 红线报错，但命令行编译能通过

原因：
- IntelliSense 没使用当前 build 目录的 `compile_commands.json`。

修复：
- 检查：
  - `.vscode/settings.json` 中 `C_Cpp.default.compileCommands`
  - `.vscode/c_cpp_properties.json` 中 `compileCommands`
- 两者都应指向 `${workspaceFolder}/build-gdb/compile_commands.json`。
- 重新执行一次 `CMake Configure (GDB)` 生成最新编译数据库。

### 5. 报错：`gdb` 找不到或无法启动

原因：
- `launch.json` 的 `miDebuggerPath` 路径不正确。

修复：
- 确认 `D:/mingw64/bin/gdb.exe` 实际存在。
- 若路径不同，更新 `.vscode/launch.json` 的 `miDebuggerPath`。

### 6. 改了 `USE_LIB_MODE` / `BUILD_SHARED` 后结果不生效

原因：
- 只做了 `build`，没有重新 `configure`。

修复：
- 修改开关后必须先执行 `CMake Configure (GDB)` 或 `CMake Init+Build (GDB)`。

### 7. 报错：`undefined reference`（链接失败）

常见原因：
- 函数声明与定义名字不一致（例如 `build_messageB` vs `build_message`）。
- `USE_LIB_MODE=ON` 时漏了 `target_link_libraries(CJAPP PRIVATE ModuleA ModuleB)`。
- 头文件目录未加入 `target_include_directories`。

排查顺序：
1. 先查函数名是否完全一致（大小写也要一致）。
2. 查 `CmakeLists.txt` 中是否把对应源文件或库加入目标。
3. 查 `include` 路径是否正确。

### 8. 输出目录没有新产物

原因：
- 目标名变化，但 `launch.json` 仍指向旧文件。
- 或构建失败但未关注前面的错误输出。

修复：
- 确认 `program` 指向 `${workspaceFolder}/out/CJAPP.exe`。
- 先单独运行一次 `cmake --build build-gdb -j`，确认无错误后再调试。

## 七、故障速查（现象 -> 一条命令）

### 1. 首次拉代码后无法调试 / 找不到构建文件

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=OFF -DBUILD_SHARED=OFF; cmake --build build-gdb -j
```

### 2. 修改了 `CmakeLists.txt` 或编译开关后，结果不生效

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=OFF -DBUILD_SHARED=OFF
```

### 3. 日常只想增量编译（不重新配置）

```powershell
cmake --build build-gdb -j
```

### 4. 切到“先编库再链接”模式

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=ON -DBUILD_SHARED=OFF; cmake --build build-gdb -j
```

### 5. 切到“动态库”模式

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=ON -DBUILD_SHARED=ON; cmake --build build-gdb -j
```

### 6. IntelliSense 红线不消失（但能编译）

```powershell
cmake -S . -B build-gdb -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

执行后在 VS Code 中运行命令：`C/C++: Reset IntelliSense Database`。

### 7. 产物异常，想从干净目录重新来

```powershell
cmake -S . -B build-gdb-clean -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DUSE_LIB_MODE=OFF -DBUILD_SHARED=OFF; cmake --build build-gdb-clean -j
```

### 8. 快速验证可执行程序是否可运行

```powershell
.\out\CJAPP.exe
```

## 八、`target_compile_definitions` 通俗解释（动态库导出/导入）

在 `CmakeLists.txt` 中你会看到：

```cmake
target_compile_definitions(ModuleA PRIVATE MODULEA_EXPORTS)
target_compile_definitions(ModuleA PUBLIC MODULEA_SHARED)
```

最简单理解：

- `MODULEA_EXPORTS`：只给 `ModuleA` 自己用（`PRIVATE`）
  - 含义：告诉编译器“我正在编译 ModuleA 这个 DLL 本体”。
  - 结果：头文件宏会展开为 `__declspec(dllexport)`，把符号导出。

- `MODULEA_SHARED`：给 `ModuleA` 和使用它的程序都用（`PUBLIC`）
  - 含义：告诉所有相关目标“当前是共享库模式”。
  - 结果：
    - 对库本身：配合 `MODULEA_EXPORTS` 走导出。
    - 对调用方（例如 `CJAPP`）：走 `__declspec(dllimport)` 导入。

一句话记忆：

- `PRIVATE MODULEA_EXPORTS` = 只有库自己知道“我是生产者（导出）”
- `PUBLIC MODULEA_SHARED` = 库和调用方都知道“现在是 DLL 模式”

如果不这样区分，常见问题是：
- 调用方拿不到 `dllimport` 信息；
- 或导出/导入方向混乱，导致链接警告或错误。
$path = 'README.md'
$text = [System.IO.File]::ReadAllText((Resolve-Path $path), [System.Text.Encoding]::Default)
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText((Resolve-Path $path), $text, $utf8NoBom)
