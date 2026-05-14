#ifndef GOMOKU_REPLAY_H
#define GOMOKU_REPLAY_H

#include "Gomoku/Board.h"
#include "Gomoku/Types.h"

#include <vector>

namespace gomoku {

/**
 * @class Replay
 * @brief 落子历史与复盘快照生成（仅适用于普通模式）。
 *
 * 普通模式一局只包含"落子"动作，可以通过记录落子序列来完整复盘。
 *
 * 进阶模式涉及消除、转化、连锁等复杂中间状态，无法仅靠落子列表还原，
 * 因此由 BoardWidget 使用 GameSnapshot 机制单独处理，不使用此类。
 */
class Replay {
public:
    /// 清空所有落子记录
    void clear();

    /// 记录一步落子
    void addMove(const Move& move);

    /// 撤销最后一步，返回 false 表示无棋可撤
    bool undoLastMove();

    /// 是否有落子记录
    bool empty() const;

    /// 已记录落子步数
    int size() const;

    /// 获取所有落子记录（const 引用）
    const std::vector<Move>& moves() const;

    /**
     * @brief 生成完整复盘快照
     * @return 从空棋盘开始，逐步落子，每一步生成一个 Board 快照
     *
     * 注意：仅包含棋盘状态，不含玩家比分、按钮状态、游戏模式等 GUI 信息。
     */
    std::vector<Board> snapshots() const;

private:
    std::vector<Move> moves_;  ///< 按时间顺序排列的落子记录
};

} // namespace gomoku

#endif // GOMOKU_REPLAY_H
