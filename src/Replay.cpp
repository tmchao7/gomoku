#include "Gomoku/Replay.h"

namespace gomoku {

/// 清空所有落子记录，开始新游戏时调用
void Replay::clear() {
    moves_.clear();
}

/**
 * @brief 记录一步落子，追加到历史末尾
 *
 * 将当前这一步棋（玩家、棋子颜色、行列位置）保存到落子列表末尾。
 * 后续悔棋时通过 undoLastMove() 删除末尾记录，复盘时通过 snapshots() 遍历重建棋盘。
 */
void Replay::addMove(const Move& move) {
    moves_.push_back(move);
}

/**
 * @brief 撤销最后一步落子（悔棋）
 *
 * 删除落子历史末尾的一条记录，配合 Board::removeStone() 使用：
 * undoLastMove() 删记录，removeStone() 清棋盘，两者共同完成悔棋。
 * @return true 撤销成功；false 历史为空，无可撤销
 */
bool Replay::undoLastMove() {
    if (moves_.empty()) {
        return false;
    }
    moves_.pop_back();
    return true;
}

/// 是否有落子记录（用于判断能否悔棋：空则不可悔）
bool Replay::empty() const {
    return moves_.empty();
}

/// 当前已记录的总落子步数
int Replay::size() const {
    return static_cast<int>(moves_.size());
}

/**
 * @brief 获取只读的落子历史
 *
 * 复盘时遍历此列表重放每一步，返回 const 引用避免外部意外修改记录。
 */
const std::vector<Move>& Replay::moves() const {
    return moves_;
}

/**
 * @brief 生成每一步之后的棋盘快照序列（供复盘动画使用）
 *
 * 遍历落子历史，从空棋盘开始逐步落子，每步保存一个 Board 副本。
 * 和 moves() 的区别：moves() 只记录"第N手下在哪"，
 * snapshots() 给出"第N步下完后棋盘长什么样"。
 * 不预存快照，使用时按需重建以节省存储空间。
 */
std::vector<Board> Replay::snapshots() const {
    std::vector<Board> result;
    Board board;

    for (const Move& move : moves_) {
        board.placeStone(move.row, move.col, move.stone);
        result.push_back(board);
    }

    return result;
}

} // namespace gomoku
