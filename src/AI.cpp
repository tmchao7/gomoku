#include "Gomoku/AI.h"

#include <cmath>

namespace gomoku {

AI::AI(Stone aiStone)
    : aiStone_(aiStone) {
}

Stone AI::aiStone() const { return aiStone_; }

Position AI::bestMove(const Board& board) const {
    return evaluateBestMove(board);
}

// ──────────────────────────────────────────────
// 单位置打分（从 stone 视角）
// ──────────────────────────────────────────────
int AI::scorePosition(const Board& board, int row, int col, Stone stone) const {
    const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    int totalScore = 0;

    for (const auto& dir : directions) {
        // 正方向连续同色子数
        int positive = 0;
        int r = row + dir[0];
        int c = col + dir[1];
        while (board.isInside(r, c) && board.at(r, c) == stone) {
            ++positive;
            r += dir[0];
            c += dir[1];
        }
        const bool blocked1 = !board.isInside(r, c) || board.at(r, c) != Stone::Empty;

        // 反方向连续同色子数
        int negative = 0;
        r = row - dir[0];
        c = col - dir[1];
        while (board.isInside(r, c) && board.at(r, c) == stone) {
            ++negative;
            r -= dir[0];
            c -= dir[1];
        }
        const bool blocked2 = !board.isInside(r, c) || board.at(r, c) != Stone::Empty;

        const int count = positive + negative;  // 不含自身
        const int openEnds = (blocked1 ? 0 : 1) + (blocked2 ? 0 : 1);

        if (count >= 4) {
            totalScore += 100000;  // 连五 / 超五
        } else if (count == 3) {
            totalScore += openEnds == 0 ? 200 : (openEnds == 1 ? 5000 : 8000);
        } else if (count == 2) {
            totalScore += openEnds == 0 ? 30 : (openEnds == 1 ? 500 : 1000);
        } else if (count == 1) {
            totalScore += openEnds == 0 ? 10 : (openEnds == 1 ? 50 : 200);
        } else {
            totalScore += openEnds * 15;  // 孤子
        }
    }

    // 星位轻微加分
    const int stars[5][2] = {{3, 3}, {3, 11}, {7, 7}, {11, 3}, {11, 11}};
    for (const auto& star : stars) {
        if (std::abs(row - star[0]) + std::abs(col - star[1]) <= 1) {
            totalScore += 5;
        }
    }

    return totalScore;
}

// ──────────────────────────────────────────────
// 启发式遍历所有空位，取攻防综合分最高者
// ──────────────────────────────────────────────
Position AI::evaluateBestMove(const Board& board) const {
    const Stone me = aiStone_;
    const Stone opp = oppositeStone(me);

    Position bestPos{7, 7};  // 默认中心
    int bestScore = -1;

    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            if (board.at(r, c) != Stone::Empty) continue;

            const int attack = scorePosition(board, r, c, me);
            const int defend = scorePosition(board, r, c, opp);
            const int total = attack + static_cast<int>(defend * 0.9);

            if (total > bestScore) {
                bestScore = total;
                bestPos = {r, c};
            }
        }
    }

    return bestPos;
}

} // namespace gomoku
