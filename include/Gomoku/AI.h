#ifndef GOMOKU_AI_H
#define GOMOKU_AI_H

#include "Gomoku/Board.h"
#include "Gomoku/Types.h"

namespace gomoku {

class AI {
public:
    enum class Difficulty { Easy, Hard };

    AI(Difficulty difficulty, Stone aiStone = Stone::White);

    /// 给定当前棋盘，返回 AI 计算出的最优落子位置
    Position bestMove(const Board& board) const;

    Difficulty difficulty() const;
    Stone aiStone() const;

private:
    Difficulty difficulty_;
    Stone aiStone_;

    /// 简单难度：遍历所有空位，对每个位置打分，取最高分
    Position evaluateBestMove(const Board& board) const;

    /// 困难难度：Minimax + α-β 剪枝，搜索深度 = 2
    Position minimaxBestMove(const Board& board) const;

    /// 对单个空位打分（从 stone 的视角），用于启发式评估
    int scorePosition(const Board& board, int row, int col, Stone stone) const;

    /// 全局面评估：对所有空位按己方和对方视角分别打分后加权求和
    int evaluate(const Board& board, Stone perspective) const;

    /// Minimax 递归搜索，maximizing = true 时是 AI 层（取最大值）
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizing) const;
};

} // namespace gomoku

#endif // GOMOKU_AI_H
