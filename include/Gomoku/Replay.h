#ifndef GOMOKU_REPLAY_H
#define GOMOKU_REPLAY_H

#include "Gomoku/Board.h"
#include "Gomoku/Types.h"

#include <vector>

namespace gomoku {

// Replay 只记录普通模式的落子序列。进阶模式有消除、转化、连锁等中间状态，
// 不能只靠落子列表还原，因此由 BoardWidget 使用 GameSnapshot 单独处理。
class Replay {
public:
    void clear();
    void addMove(const Move& move);
    bool undoLastMove();
    bool empty() const;
    int size() const;
    const std::vector<Move>& moves() const;

    // 从 moves_ 重新播放生成每一步棋盘状态；不保存按钮、比分、状态机等 GUI 状态。
    std::vector<Board> snapshots() const;

private:
    std::vector<Move> moves_;
};

} // namespace gomoku

#endif // GOMOKU_REPLAY_H
