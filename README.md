# C++ 五子棋游戏系统课程设计

这是一个完全基于 C++17 标准库实现的控制台五子棋游戏系统，支持 macOS 和 Windows 编译运行。

## 功能

- 欢迎界面和主菜单
- 玩家登录界面
- 游戏规则说明
- 按键功能说明
- 15 x 15 五子棋对战
- 横向、纵向、两种斜向五连胜负判断
- 非法落子检测
- 悔棋功能
- 游戏结束后返回赢家信息
- 游戏结束后复盘
- 核心规则测试程序

## 项目结构

```text
.
├── CMakeLists.txt
├── README.md
├── include/Gomoku/
│   ├── Board.h
│   ├── Game.h
│   ├── Player.h
│   ├── Replay.h
│   ├── Types.h
│   └── UI.h
├── src/
│   ├── Board.cpp
│   ├── Game.cpp
│   ├── Player.cpp
│   ├── Replay.cpp
│   ├── Types.cpp
│   ├── UI.cpp
│   └── main.cpp
└── tests/
    └── test_core.cpp
```

## macOS 编译运行

要求：

- CMake 3.16 或更高版本
- 支持 C++17 的 Clang

命令：

```bash
cmake -S . -B build
cmake --build build
./build/gomoku
```

运行测试：

```bash
./build/gomoku_tests
```

## Windows 编译运行

推荐方式一：Visual Studio 2022 + CMake

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\gomoku.exe
```

运行测试：

```powershell
.\build\Debug\gomoku_tests.exe
```

推荐方式二：MinGW-w64 + CMake

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\gomoku.exe
```

运行测试：

```powershell
.\build\gomoku_tests.exe
```

## 游戏操作

主菜单：

- `1`：开始游戏
- `2`：阅读游戏规则
- `3`：查看按键说明
- `4`：退出游戏

对局中：

- 输入 `行 列` 落子，例如 `8 8`
- 输入 `u` 悔棋
- 输入 `q` 认输并返回结算

棋盘坐标范围是 `1` 到 `15`。黑方使用 `X`，白方使用 `O`，空位使用 `.`。

## 设计说明

核心设计目标是把“规则逻辑”和“控制台显示”分离：

- `Board` 只负责棋盘状态、落子合法性、胜负判断。
- `Replay` 只负责记录和回放落子历史。
- `Game` 负责组织完整对局流程。
- `UI` 负责菜单、说明和终端输出。

这样做的好处是：胜负判断、悔棋、复盘这些核心逻辑可以脱离人工输入进行测试，课程设计答辩时也更容易解释模块职责。

## 测试覆盖

`tests/test_core.cpp` 覆盖：

- 横向五连
- 纵向五连
- 两种斜向五连
- 越界落子
- 重复落子
- 悔棋恢复棋盘状态
- 复盘记录和撤销

## 课程设计说明

本项目只使用 C++ 标准库，不依赖平台专有图形库，因此适合在 macOS 开发、Windows 运行和展示。
