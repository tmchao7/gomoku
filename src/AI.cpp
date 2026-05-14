#include "Gomoku/AI.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gomoku {

AI::AI(Difficulty difficulty, Stone aiStone)
    : difficulty_(difficulty), aiStone_(aiStone) {
}

AI::Difficulty AI::difficulty() const { return difficulty_; }
Stone AI::aiStone() const { return aiStone_; }

// ──────────────────────────────────────────────
// 入口：根据难度选择不同的决策算法
// ──────────────────────────────────────────────
Position AI::bestMove(const Board& board) const {
    if (difficulty_ == Difficulty::Easy) {
        return evaluateBestMove(board);
    }
    return minimaxBestMove(board);
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
// 全局评估（从 perspective 视角）
// ──────────────────────────────────────────────
int AI::evaluate(const Board& board, Stone perspective) const {
    const Stone opponent = oppositeStone(perspective);
    int score = 0;

    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            if (board.at(r, c) != Stone::Empty) continue;

            score += scorePosition(board, r, c, perspective);
            score += static_cast<int>(scorePosition(board, r, c, opponent) * 0.9);
        }
    }

    return score;
}

// ──────────────────────────────────────────────
// 简单难度：启发式遍历所有空位取最高分
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

// ──────────────────────────────────────────────
// Minimax 递归（α-β 剪枝，搜索深度 2）
// ──────────────────────────────────────────────
int AI::minimax(Board& board, int depth, int alpha, int beta, bool maximizing) const {
    const Stone perspective = maximizing ? aiStone_ : oppositeStone(aiStone_);

    if (depth == 0) {
        return evaluate(board, aiStone_);
    }

    // 检查终止：有人五连
    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            if (board.at(r, c) != Stone::Empty && board.hasFiveInRow(r, c)) {
                return board.at(r, c) == aiStone_ ? 1000000 : -1000000;
            }
        }
    }

    if (maximizing) {
        int maxScore = std::numeric_limits<int>::min();
        for (int r = 0; r < kBoardSize; ++r) {
            for (int c = 0; c < kBoardSize; ++c) {
                if (!board.isEmpty(r, c)) continue;

                board.placeStone(r, c, perspective);
                const int score = minimax(board, depth - 1, alpha, beta, false);
                board.removeStone(r, c);

                if (score > maxScore) maxScore = score;
                if (score > alpha) alpha = score;
                if (beta <= alpha) break;
            }
            if (beta <= alpha) break;
        }
        return maxScore;
    } else {
        int minScore = std::numeric_limits<int>::max();
        for (int r = 0; r < kBoardSize; ++r) {
            for (int c = 0; c < kBoardSize; ++c) {
                if (!board.isEmpty(r, c)) continue;

                board.placeStone(r, c, perspective);
                const int score = minimax(board, depth - 1, alpha, beta, true);
                board.removeStone(r, c);

                if (score < minScore) minScore = score;
                if (score < beta) beta = score;
                if (beta <= alpha) break;
            }
            if (beta <= alpha) break;
        }
        return minScore;
    }
}

// ──────────────────────────────────────────────
// 困难难度入口：在 minimax 顶层遍历所有空位
// ──────────────────────────────────────────────
Position AI::minimaxBestMove(const Board& board) const {
    Board mutableBoard = board;
    Position bestPos{7, 7};
    int bestScore = std::numeric_limits<int>::min();

    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            if (!mutableBoard.isEmpty(r, c)) continue;

            mutableBoard.placeStone(r, c, aiStone_);
            const int score = minimax(mutableBoard, 2,
                                      std::numeric_limits<int>::min(),
                                      std::numeric_limits<int>::max(),
                                      false);
            mutableBoard.removeStone(r, c);

            if (score > bestScore) {
                bestScore = score;
                bestPos = {r, c};
            }
        }
    }

    return bestPos;
}

} // namespace gomoku
