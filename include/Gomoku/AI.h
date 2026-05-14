#ifndef GOMOKU_AI_H
#define GOMOKU_AI_H

#include "Gomoku/Board.h"
#include "Gomoku/Types.h"

namespace gomoku {

/**
 * @brief 人机对战 AI，使用启发式打分策略选择落子位置
 *
 * 不依赖任何 GUI 框架，可被命令行/测试程序复用。
 * 评估逻辑：对每个空位从进攻和防守两个角度打分，取最高分。
 */
class AI {
public:
    AI(Stone aiStone = Stone::White);

    /// 给定当前棋盘，返回 AI 计算出的最优落子位置
    Position bestMove(const Board& board) const;

    Stone aiStone() const;

private:
    Stone aiStone_;

    /// 遍历所有空位，对每个位置打分，取最高分
    Position evaluateBestMove(const Board& board) const;

    /// 对单个空位打分（从 stone 的视角），用于启发式评估
    int scorePosition(const Board& board, int row, int col, Stone stone) const;
};

} // namespace gomoku

#endif // GOMOKU_AI_H
