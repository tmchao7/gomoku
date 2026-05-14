# 五子棋项目结构与运行逻辑拆解

本文用于帮助阅读和维护本项目源码。项目本质上是一个使用 C++17 + Qt6 Widgets 实现的图形界面五子棋游戏，核心设计思路是把“棋盘规则”和“界面交互”分开：规则层可以脱离 GUI 独立测试，界面层负责绘制、输入、状态切换和复盘控制。

## 1. 项目目录结构

```text
.
├── CMakeLists.txt              # CMake 构建入口，定义核心库、GUI 程序、测试程序
├── Makefile                    # 对常用 cmake/build/test/run/clean 命令做一层封装
├── README.md                   # 项目功能、规则、构建运行说明
├── include/Gomoku/             # 纯 C++ 核心逻辑头文件，对外暴露规则层接口
│   ├── Types.h                 # 棋子、坐标、游戏模式、落子记录等基础类型
│   ├── Board.h                 # 棋盘状态和五连判断等核心规则
│   ├── Replay.h                # 普通模式落子历史和复盘快照生成
│   └── Player.h                # 玩家姓名和棋子颜色
├── src/                        # 具体实现文件
│   ├── Types.cpp
│   ├── Board.cpp
│   ├── Replay.cpp
│   ├── Player.cpp
│   ├── boardwidget.h           # Qt 主窗口类声明
│   ├── boardwidget.cpp         # Qt 绘制、鼠标事件、状态机、按钮和复盘逻辑
│   └── main.cpp                # 程序入口，创建 QApplication 和 BoardWidget
├── tests/
│   └── test_core.cpp           # 不依赖 Qt 的核心规则测试
└── docs/
    ├── design.md               # 设计说明
    └── project-guide.md        # 当前这份项目阅读指南
```

阅读代码时可以先看 `include/Gomoku/Types.h` 和 `include/Gomoku/Board.h`，理解棋子、坐标和棋盘规则；再看 `Replay`、`Player`；最后读 `src/boardwidget.cpp`。`BoardWidget` 文件最长，因为它同时承担 GUI 绘制、输入处理、按钮状态、普通/进阶模式流程和复盘控制。

## 2. 构建目标关系

`CMakeLists.txt` 定义了三个目标：

```text
gomoku_core  (static library)
├── Types.cpp
├── Board.cpp
├── Player.cpp
└── Replay.cpp

gomoku       (Qt GUI executable)
├── main.cpp
└── boardwidget.cpp
    └── links: gomoku_core + Qt6::Widgets

gomoku_tests (core test executable)
└── test_core.cpp
    └── links: gomoku_core
```

这个结构的关键价值是：`Board`、`Replay`、`Player`、`Types` 不依赖 Qt，因此核心规则可以被 `gomoku_tests` 直接测试；Qt 相关代码只集中在 `BoardWidget` 和 `main.cpp` 中。

常用命令已经被 `Makefile` 封装：

```bash
make build      # 配置并构建
make test       # 构建并运行核心测试
make run        # 构建并运行图形界面程序
make clean      # 清理 build 内产物
make distclean  # 删除整个 build 目录
```

## 3. 程序启动流程

程序入口在 `src/main.cpp`：

```text
main.cpp
  -> 创建 QApplication
  -> 创建 BoardWidget
  -> 设置窗口标题并 show()
  -> app.exec() 进入 Qt 事件循环
```

`BoardWidget` 构造时会完成几件事：

1. 固定窗口大小为 `640 x 680`。
2. 创建悔棋、认输、复盘、重新开始四个按钮。
3. 创建自动复盘用的 `QTimer`，间隔为 700ms。
4. 用 `QTimer::singleShot(0, ...)` 在窗口进入事件循环后调用 `startNewGame()`。

`startNewGame()` 会依次完成：

```text
输入黑方姓名
输入白方姓名
选择普通模式 / 进阶模式
清空棋盘和历史
黑方先手
保存初始快照
刷新按钮和界面
```

## 4. 核心运行逻辑

用户操作入口是 `BoardWidget::mousePressEvent()`。Qt 收到鼠标点击后，程序会按下面的路径处理：

```text
鼠标点击
  -> 忽略非左键、游戏结束、复盘模式下的点击
  -> pixelToGrid() 将像素坐标转换为棋盘 row/col
  -> 根据 gameMode_ 分流
       普通模式: placeStone(row, col)
       进阶模式: handleAdvancedClick(row, col)
```

坐标转换由 `pixelToGrid()` 完成。它不是要求玩家精准点中交叉点，而是允许半个格子左右的曼哈顿距离容差，这样鼠标点击体验更自然。

界面刷新主要靠 `update()` 触发 Qt 重绘，最终进入 `paintEvent()`：

```text
paintEvent()
  -> drawBoard()           绘制棋盘线和星位
  -> drawStones()          绘制黑白棋子
  -> drawSelectableLines() 进阶模式下绘制可消除五连
  -> drawStatusText()      绘制当前回合、比分、复盘提示
```

## 5. 数据与存储方式

本项目没有数据库，也没有把棋局写入文件。所有与五子棋游戏有关的状态都保存在内存中。

### 5.1 棋盘状态：`Board::cells_`

`Board` 是规则层最核心的类，内部用一个固定大小二维数组保存棋盘：

```cpp
std::array<std::array<Stone, kBoardSize>, kBoardSize> cells_;
```

其中 `kBoardSize = 15`，每个格子的值是：

```text
Stone::Empty  空位
Stone::Black  黑子
Stone::White  白子
```

所有落子、移除、替换、查询和五连判断，最终都围绕 `cells_` 进行。

### 5.2 玩家与当前状态：`BoardWidget`

`BoardWidget` 负责维护一局游戏的交互状态：

```text
players_       黑白双方玩家姓名和棋子颜色
currentStone_  当前该谁下
gameMode_      普通模式或进阶模式
scores_        进阶模式双方得分，黑方下标 0，白方下标 1
state_         进阶模式当前交互阶段
gameOver_      当前游戏是否结束
replayMode_    当前是否正在复盘
```

这些状态只存在于程序运行期间。关闭程序后，棋局不会自动保存。

### 5.3 普通模式历史：`Replay::moves_`

普通模式的历史记录是一个落子序列：

```cpp
std::vector<Move> moves_;
```

每个 `Move` 保存：

```text
row         行
col         列
stone       黑子或白子
playerName 该步玩家姓名
```

普通模式悔棋时，只需要删除最后一步落子并从棋盘移除对应棋子。普通模式复盘时，`Replay::snapshots()` 会从空棋盘开始重新播放 `moves_`，生成每一步棋盘快照。

### 5.4 进阶模式历史：`BoardWidget::history_`

进阶模式不能只记录落子，因为一次得分流程可能包含：

```text
落子 -> 选择五连 -> 消除五子 -> 得分 -> 转化对方棋子 -> 可能继续连锁
```

所以进阶模式使用完整快照：

```cpp
struct GameSnapshot {
    Board board;
    Stone currentStone;
    std::array<int, 2> scores;
    InteractionState state;
    bool gameOver;
    QString label;
};
```

每个快照保存棋盘、当前玩家、比分、状态机阶段和游戏是否结束。进阶模式悔棋时直接弹出快照栈顶，再恢复上一个快照；进阶模式复盘也基于这些快照展示历史过程。

## 6. 普通模式流程

普通模式入口是 `BoardWidget::placeStone()`：

```text
placeStone(row, col)
  -> board_.placeStone() 尝试落子
  -> replay_.addMove() 记录落子
  -> board_.hasFiveInRow() 判断是否形成五连
       是: gameOver_ = true，弹窗提示获胜
       否: 继续判断 board_.isFull()
            满: 平局
            未满: switchPlayer()
```

`Board::hasFiveInRow()` 只检查最后落子点所在的四个方向：

```text
水平
垂直
主对角线
副对角线
```

它不会扫描整张棋盘。这样更符合游戏判断逻辑，也更高效。

## 7. 进阶模式状态机

进阶模式由 `BoardWidget::InteractionState` 控制：

```text
Playing          正常落子
SelectingLine    已形成五连，等待玩家选择要消除的五连端点
ConvertingStone  已消除并得分，等待玩家选择一颗对方棋子进行转化
GameOver         游戏结束
```

主流程如下：

```text
Playing
  -> 当前玩家落子
  -> beginLineSelection()
       没有五连: finishAdvancedAction()，切换玩家
       有五连: 进入 SelectingLine

SelectingLine
  -> 玩家点击同一条五连的两个端点
  -> resolveSelectedLine()
       移除五颗棋子
       当前玩家得 1 分
       分数达到 3: GameOver
       没有对方棋子: finishAdvancedAction()
       有对方棋子: 进入 ConvertingStone

ConvertingStone
  -> 玩家点击一颗对方棋子
  -> replaceStone() 转化为己方棋子
  -> beginLineSelection()
       若转化后又成五，继续 SelectingLine
       若没有成五，finishAdvancedAction()，切换玩家
```

这里最重要的是：进阶模式中，当前玩家在完成“消除 + 转化 + 可能连锁”之前不会切换。只有 `finishAdvancedAction()` 被调用后，才会进入下一方回合。

## 8. 五连候选与端点选择

进阶模式中，一个落子可能同时形成多条五连，例如横向和纵向同时成五。`Board::fiveLineCandidates()` 会找出所有包含触发位置的连续五子候选。

它的思路是：

1. 对四个方向分别处理。
2. 先退到该方向连续同色棋子的起点。
3. 收集整条连续同色线段。
4. 对长度大于等于 5 的线段滑动窗口。
5. 只保留包含本次触发位置的 5 子窗口。

玩家选择两个端点时，`findFiveLineByEndpoints()` 会校验：

```text
两个点必须在水平、垂直或对角线上
两个点最大坐标距离必须正好是 4
中间 5 个位置必须全部是当前玩家棋子
```

这样可以避免玩家误选不连续、长度错误或非同色的线段。

## 9. 悔棋与复盘机制

普通模式和进阶模式的悔棋机制不同。

### 普通模式悔棋

普通模式只有单步落子，因此悔棋逻辑很简单：

```text
取 replay_.moves().back()
从 board_ 移除该位置棋子
replay_.undoLastMove()
currentStone_ 回到最后落子的棋子颜色
```

### 进阶模式悔棋

进阶模式中一次操作可能产生多个中间状态，因此使用快照栈：

```text
history_.pop_back()
restoreSnapshot(history_.back())
```

恢复快照时会同时恢复棋盘、当前玩家、比分、状态机阶段和游戏结束标记。

### 复盘显示

复盘时不会直接改真实棋盘 `board_`，而是使用 `replayBoard_` 作为显示棋盘：

```text
真实对局棋盘: board_
复盘显示棋盘: replayBoard_
```

`drawStones()` 绘制棋子时会调用 `displayedStoneAt()`：

```text
如果 replayMode_ 为 true，读取 replayBoard_
否则读取 board_
```

这样退出复盘后，真实棋局状态仍然保留。

## 10. 推荐阅读顺序

建议按下面顺序读代码：

1. `include/Gomoku/Types.h`
   - 先理解 `Stone`、`GameMode`、`Position`、`Move`。
2. `include/Gomoku/Board.h` 和 `src/Board.cpp`
   - 理解棋盘如何存储，落子、移除、五连判断如何实现。
3. `include/Gomoku/Replay.h` 和 `src/Replay.cpp`
   - 理解普通模式历史记录和复盘快照生成。
4. `include/Gomoku/Player.h` 和 `src/Player.cpp`
   - 理解玩家信息只是姓名和棋子颜色的简单封装。
5. `src/boardwidget.h`
   - 先看成员变量，理解 GUI 层保存了哪些状态。
6. `src/boardwidget.cpp`
   - 按“启动 -> 绘制 -> 鼠标点击 -> 普通模式 -> 进阶模式 -> 悔棋 -> 复盘”的顺序阅读。
7. `tests/test_core.cpp`
   - 对照测试理解哪些规则已经被验证。

如果只想快速理解项目主线，可以重点看：

```text
main.cpp
BoardWidget::startNewGame()
BoardWidget::mousePressEvent()
BoardWidget::placeStone()
BoardWidget::handleAdvancedClick()
Board::hasFiveInRow()
Board::fiveLineCandidates()
BoardWidget::undoLastMove()
BoardWidget::enterReplayMode()
```
