# Qt6 五子棋游戏系统设计文档

## 一、需求分析

本系统实现一个基于 C++17 + Qt6 Widgets 的图形界面五子棋游戏。玩家通过鼠标点击 15 x 15 棋盘交叉点直接落子，黑白双方轮流下棋。程序需要阻止重复落子，并在横向、纵向或斜向形成五子连珠时弹窗提示赢家。

## 二、总体设计

系统采用“规则逻辑”和“图形界面”分离的结构：

```text
main -> BoardWidget -> Board
                    -> Types
                    -> Replay
```

`main.cpp` 只负责启动 Qt 应用；`BoardWidget` 负责绘制和交互；`Board` 负责棋盘状态与胜负判断。

## 三、模块设计

### 1. Board 棋盘模块

职责：

- 保存 15 x 15 棋盘状态。
- 判断坐标是否越界。
- 判断指定位置是否为空。
- 执行落子和撤销落子。
- 判断棋盘是否已满。
- 判断指定落子是否形成五连。

胜负判断方法：

每次落子后，只需要以该落子为中心检查四个方向：

- 横向：`(0, 1)`
- 纵向：`(1, 0)`
- 主对角线：`(1, 1)`
- 副对角线：`(1, -1)`

在每个方向上分别向正反两个方向统计连续同色棋子数量，总数大于等于 5 即获胜。

### 2. BoardWidget 图形界面模块

职责：

- 使用 `QPainter` 绘制棋盘线、星位和棋子。
- 使用 `mousePressEvent` 接收鼠标点击。
- 将鼠标坐标转换成棋盘行列。
- 调用 `Board::placeStone` 完成落子。
- 黑白双方轮流切换。
- 调用 `Board::hasFiveInRow` 判断胜利。
- 使用 `QMessageBox` 弹窗提示赢家。

### 3. Types 公共类型模块

职责：

- 定义棋盘大小。
- 定义棋子类型：空位、黑子、白子。
- 定义落子记录结构。
- 提供棋子名称、棋子字符和对方棋子转换函数。

### 4. Replay 复盘模块

职责：

- 保存每一步落子记录。
- 支持撤销最后一步记录。
- 支持生成历史棋盘快照。

当前 GUI 版本尚未接入复盘按钮，但该模块保留给后续扩展使用。

## 四、关键交互流程

```text
鼠标点击
  -> BoardWidget::mousePressEvent
  -> 像素坐标转换为棋盘 row / col
  -> Board::placeStone(row, col, currentStone)
  -> Board::hasFiveInRow(row, col)
  -> 若胜利，QMessageBox 提示赢家
  -> 否则切换当前玩家
```

## 五、构建设计

项目使用 CMake 管理构建：

- `gomoku_core`：棋盘规则、类型、复盘等核心逻辑。
- `gomoku`：Qt6 Widgets 图形界面可执行程序。
- `gomoku_tests`：核心规则测试程序。

Qt 依赖通过：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets)
```

引入，并链接到 GUI 目标：

```cmake
target_link_libraries(gomoku PRIVATE gomoku_core Qt6::Widgets)
```

## 六、测试设计

测试程序位于 `tests/test_core.cpp`，主要验证：

- 横向五连胜利。
- 纵向五连胜利。
- 两种斜向五连胜利。
- 越界落子失败。
- 重复落子失败。
- 悔棋后棋盘恢复为空。
- 复盘记录与撤销同步。

## 七、扩展方向

后续可以继续添加：

- 悔棋按钮。
- 重新开始按钮。
- 游戏状态栏。
- 人机对战 AI。
- 复盘回放。
- 禁手规则。
- 落子音效。
