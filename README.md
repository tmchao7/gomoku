# Qt6 五子棋游戏

这是一个使用 C++17 + Qt6 Widgets 实现的图形界面五子棋项目。玩家可以直接用鼠标点击棋盘交叉点落子，黑白双方轮流下棋，程序会自动检测横向、纵向和两种斜向的五子连珠胜利。

## 功能

- Qt6 Widgets 图形界面
- 启动时注册黑方、白方玩家姓名
- 15 x 15 标准五子棋棋盘
- 鼠标点击棋盘交叉点直接落子
- 黑白双方轮流下棋
- 黑子、白子清晰绘制
- 已有棋子位置不可重复落子
- 支持悔棋、认输和重新开始
- 游戏结束后支持逐步复盘
- 横向、纵向、斜向五连胜负判断
- 获胜后弹窗提示“阵营 + 玩家姓名”
- 核心规则和 GUI 显示分离，方便继续扩展悔棋、人机对战、复盘等功能

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

如果你已经通过 Homebrew 安装 Qt6，通常可以直接构建：

```bash
cmake -S . -B build
cmake --build build
./build/gomoku
```

如果 CMake 找不到 Qt6，可以显式指定 Homebrew Qt 路径：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
./build/gomoku
```

运行核心规则测试：

```bash
./build/gomoku_tests
```

## 游戏操作

- 黑方先手。
- 启动游戏后先输入黑方姓名、白方姓名。
- 用鼠标点击棋盘交叉点落子。
- 点击已有棋子的位置不会重复落子。
- 点击“悔棋”撤销最后一步落子。
- 点击“认输”后，当前回合玩家认输，对方获胜。
- 游戏结束后可以选择立即复盘，也可以点击“复盘”按钮进入复盘模式。
- 复盘模式下使用“上一步 / 下一步 / 退出复盘”查看历史棋局。
- 点击“重新开始”重置棋盘并重新输入玩家姓名。
- 某一方形成横向、纵向或斜向连续五子后，弹窗提示“黑方/白方：玩家姓名 获胜”。
- 获胜后棋盘停止接收新的落子。

## 设计说明

核心设计目标是把“规则逻辑”和“图形界面”分离：

- `Board` 负责棋盘状态、落子合法性、撤销落子、胜负判断。
- `Types` 定义棋子颜色、棋盘大小和落子记录。
- `Replay` 保留复盘扩展能力。
- `BoardWidget` 负责 Qt 绘制、鼠标点击、玩家注册、悔棋、认输、复盘、回合切换和胜利弹窗。
- `main.cpp` 只负责启动 `QApplication` 和显示主窗口。

这种结构的好处是：五子棋规则可以脱离 GUI 单独测试；后续要加悔棋、人机对战、AI 搜索或网络对战时，可以继续复用 `Board`。

## 测试覆盖

`tests/test_core.cpp` 覆盖：

- 横向五连
- 纵向五连
- 两种斜向五连
- 越界落子
- 重复落子
- 悔棋恢复棋盘状态
- 复盘记录和撤销
