# Qt6 五子棋游戏

这是一个使用 C++17 + Qt6 Widgets 实现的图形界面五子棋项目。项目支持普通模式和进阶模式：普通模式是传统五连即胜；进阶模式加入“五连消除、得分、转化对方棋子、连锁判定”等规则。

## 功能特性

- Qt6 Widgets 图形界面
- 15 x 15 标准棋盘
- 鼠标点击棋盘交叉点直接落子
- 启动时注册黑方、白方玩家姓名
- 启动时选择普通模式或进阶模式
- 黑白双方轮流下棋，黑方先手
- 已有棋子位置不可重复落子
- 支持悔棋、认输、重新开始
- 游戏结束后支持自动复盘和手动复盘
- 获胜弹窗显示阵营和玩家姓名
- 核心规则和 GUI 显示分离，便于继续扩展 AI、禁手、网络对战等功能

## 游戏规则

### 普通模式

- 黑方先手。
- 双方轮流落子。
- 任意一方形成横向、纵向或斜向连续五子后立即获胜。
- 游戏结束后可以进入复盘。

### 进阶模式

- 黑方先手。
- 双方轮流落子。
- 当前玩家落子后，如果没有形成五连，则切换到对方。
- 如果形成五连，系统会用彩色线段标出可消除五连。
- 玩家需要点击同一条彩色五连线段的两个端点，选择要消除的五连。
- 消除后当前玩家得 1 分。
- 得分后，当前玩家需要点击一颗对方棋子，将其转化为己方棋子。
- 如果棋盘上没有对方棋子，则跳过转化。
- 转化后如果再次形成五连，会继续进入五连选择流程。
- 连锁流程全部结束后，切换到对方。
- 先得 3 分者获胜。

## 游戏操作

- 点击棋盘交叉点落子。
- 点击“悔棋”撤销上一步状态。
- 点击“认输”结束当前对局，对方获胜。
- 点击“复盘”进入复盘模式，并选择“自动复盘”或“手动复盘”。
- 自动复盘会像动画一样按时间顺序播放历史棋局，可暂停、继续或退出。
- 手动复盘使用“上一步 / 下一步 / 退出复盘”查看历史棋局。
- 点击“重新开始”重置棋盘，并重新输入玩家姓名和选择模式。

## 项目结构

```text
.
├── CMakeLists.txt
├── README.md
├── include/Gomoku/
│   ├── Board.h
│   ├── Player.h
│   ├── Replay.h
│   └── Types.h
├── src/
│   ├── Board.cpp
│   ├── Player.cpp
│   ├── Replay.cpp
│   ├── Types.cpp
│   ├── boardwidget.h
│   ├── boardwidget.cpp
│   └── main.cpp
└── tests/
    └── test_core.cpp
```

## macOS 编译运行

要求：

- CMake 3.16 或更高版本
- 支持 C++17 的 Clang
- Qt6 Widgets

如果通过 Homebrew 安装 Qt6：

```bash
brew install qt
```

构建运行：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
./build/gomoku
```

运行测试：

```bash
./build/gomoku_tests
```

## Windows 编译运行

推荐环境：

- Windows 10 / 11
- Visual Studio 2022，安装“使用 C++ 的桌面开发”
- CMake 3.16 或更高版本
- Qt 6.x，安装 MSVC 版本组件，例如 `msvc2022_64`

假设 Qt 安装路径为：

```text
C:\Qt\6.6.3\msvc2022_64
```

Visual Studio 生成器构建：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\msvc2022_64"
cmake --build build --config Release
.\build\Release\gomoku.exe
```

运行测试：

```powershell
.\build\Release\gomoku_tests.exe
```

如果希望把 `gomoku.exe` 拷贝到其他目录或直接双击运行，建议使用 Qt 自带的部署工具复制依赖：

```powershell
C:\Qt\6.6.3\msvc2022_64\bin\windeployqt.exe .\build\Release\gomoku.exe
```

如果当前 PowerShell 已经把 Qt 的 `bin` 目录加入 `PATH`，也可以直接运行：

```powershell
windeployqt .\build\Release\gomoku.exe
```

如果使用 Qt Creator，也可以直接打开项目根目录的 `CMakeLists.txt`，选择 Qt6 MSVC Kit 后构建运行。

## VSCode 配置说明

项目已开启：

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

macOS 下 `.vscode/settings.json` 会让 VSCode C/C++ 插件读取：

```text
build/compile_commands.json
```

如果 VSCode 仍提示找不到 `QWidget`、`QPainter` 等 Qt 头文件：

1. 先重新配置 CMake。
2. 执行 `Developer: Reload Window`。
3. 执行 `C/C++: Reset IntelliSense Database`。

Windows 下如果 Qt 不在默认路径，需要把 `.vscode/settings.json` 中的 `CMAKE_PREFIX_PATH` 改成自己的 Qt MSVC 路径。

## 设计说明

核心设计目标是把规则逻辑和图形界面分离：

- `Board` 负责棋盘状态、落子合法性、胜负判断、五连候选查找、批量移除和棋子转化。
- `Types` 定义棋子颜色、棋盘大小、游戏模式和落子记录。
- `Replay` 保留普通模式复盘能力。
- `BoardWidget` 负责 Qt 绘制、鼠标点击、玩家注册、模式选择、悔棋、认输、复盘和胜利弹窗。
- `main.cpp` 只负责启动 `QApplication` 和显示主窗口。

这样做的好处是：核心规则可以脱离 GUI 单独测试，后续扩展 AI、人机对战、禁手规则时更容易维护。

## 测试覆盖

`tests/test_core.cpp` 覆盖：

- 横向五连
- 纵向五连
- 两种斜向五连
- 越界落子
- 重复落子
- 悔棋恢复棋盘状态
- 复盘记录和撤销
- 五连候选识别
- 按端点查找合法五连
- 批量移除棋子
- 替换棋子颜色
