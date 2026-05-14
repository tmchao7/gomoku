# 五子棋人机对战模式 — 设计文档

**日期**：2026-05-14  
**分支**：`feat--newMode`

## 一、目标

在现有的"玩家对战"基础上，新增"人机对战"模式：
- 玩家执黑先行，AI执白后行
- 简单难度：规则型启发式打分策略
- 困难难度：Minimax + α-β 剪枝，搜索深度 2

## 二、架构设计

新增 `AI` 类，位于核心库 `gomoku_core`，不依赖 Qt：

```
include/Gomoku/AI.h     ← 声明 AI 接口
src/AI.cpp               ← 两种策略的具体实现
```

由 `BoardWidget` 在回合切换时调用：

```
玩家落子 → placeStone() → 检查五连/平局 → AI 回合 → aiMove() → 检查五连/平局 → 玩家回合
```

## 三、AI 类接口

```cpp
class AI {
public:
    enum class Difficulty { Easy, Hard };

    AI(Difficulty difficulty, Stone aiStone = Stone::White);

    // 给定当前棋盘，返回 AI 计算出的最优落子位置
    Position bestMove(const Board& board) const;

    Difficulty difficulty() const;
    Stone aiStone() const;

private:
    Difficulty difficulty_;
    Stone aiStone_;

    // 简单难度：对每个空位评分，取最高分位置
    Position evaluateBestMove(const Board& board) const;

    // 困难难度：Minimax + α-β 剪枝
    Position minimaxBestMove(const Board& board) const;

    // 局面评估函数（两种难度共用）
    int evaluate(const Board& board, Stone perspective) const;

    // 对单一位置的攻防评分
    int scorePosition(const Board& board, int row, int col, Stone stone) const;

    // Minimax 递归搜索
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizing) const;
};
```

## 四、评估函数设计（scorePosition）

对单个空位打分，衡量该位置对指定颜色的"价值"。考察四个方向，每方向统计该位置两侧连续同色子数：

| 模式 | 说明 | 得分 |
|---|---|---|
| 四连（一侧 4 + 另一侧 0） | 堵/进攻优先 | +5000 |
| 活三（两侧合计 3，两端都有活路） |  | +1000 |
| 活二 |  | +200 |
| 单子连接 |  | +50 |

plus 额外项：星位（棋盘中心及经典星位）有少许加分。

评估函数合并"己方得分 + 0.9 × 对方得分"，兼顾进攻与防守。

## 五、困难模式 Minimax

- 搜索深度固定为 2（两步前瞻：AI 走一步 → 玩家应对一步）
- α-β 剪枝减少无用分支
- `maximizing` = true 时（AI 层）找最大值，false 时（玩家层）找最小值
- 叶节点用 `evaluate()` 估值

剪枝前备选步数约 225，深度 2 可搜完；深度 4 以上需要加候选步前筛（本次不做）。

## 六、模式选择流程改动

`askGameMode()` 中 `QStringList` 改为四项，一步完成所有选择：

```
"玩家对战-普通" → GameMode::Classic，无 AI
"玩家对战-进阶" → GameMode::AdvancedCapture，无 AI（保持现有逻辑不变）
"人机-简单"     → GameMode::Classic，AI Easy
"人机-困难"     → GameMode::Classic，AI Hard
```

## 七、BoardWidget 集成改动

新增成员：
```cpp
std::unique_ptr<gomoku::AI> ai_;  // 非空时为人机模式
```

新增函数：
```cpp
void triggerAITurn();       // 玩家落子后延迟触发 AI 落子
void executeAIMove();       // 实际执行 AI 落子 + 记录 Replay + 判断胜负
```

`placeStone()` 末尾逻辑：
```cpp
if (ai_) {
    triggerAITurn();  // AI 走完后再 switchPlayer()
} else {
    switchPlayer();
}
```

关键联动：
- AI 落子通过 `executeAIMove()` 执行，调用 `board_.placeStone()` 和 `replay_.addMove()`，与普通模式玩家的落子流程一致，复盘系统无需改造。
- AI 落子使用 `QTimer::singleShot(300ms)` 延迟触发，给玩家一个"AI在思考"的感知缓冲，避免瞬间落子。

`undoLastMove()` 适配：
- 人机模式下依次撤销两步：先 `removeStone()` + `undoLastMove()` 撤销 AI 的落子，再撤销玩家上一步。
- 只有玩家回合才能点悔棋（AI 思考期间按钮禁用）。

`startNewGame()` 中根据 `askGameMode()` 的返回值决定 `ai_` 的创建/销毁。

## 八、测试计划

在 `tests/test_core.cpp` 中添加：
- `test_ai_evaluate` — 验证评估函数对已知局面的打分
- `test_ai_easy_move` — 简单 AI 在有明显活三时应优先防守
- `test_ai_hard_move` — 困难 AI 在能连五时应直接连五
- `test_ai_block_win` — AI 应在对手即将连五时堵

## 九、不做的事情

- 不支持 AI 下的进阶模式（消除/转化逻辑复杂，搜索空间大）
- 不实现深度 4 以上的搜索（需候选步前筛 + PB 表）
- 不做开局库
- 不实现置换表（Transposition Table）
