#ifndef GOMOKU_TYPES_H
#define GOMOKU_TYPES_H

#include <string>

namespace gomoku {

/**
 * @brief 棋盘尺寸 = 15×15
 */
constexpr int kBoardSize = 15;

/**
 * @brief 棋子/格子状态
 */
enum class Stone {
    Empty = 0,  ///< 空格
    Black,      ///< 黑子
    White       ///< 白子（黑方先走）
};

/**
 * @brief 游戏模式
 */
enum class GameMode {
    Classic,         ///< 普通模式：先连成五子者获胜
    AdvancedCapture  ///< 进阶模式：通过消除/转化累计 3 分获胜
};

/**
 * @brief 棋盘坐标：先行后列，(0,0) = 左上角
 */
struct Position {
    int row;
    int col;
};

/// 坐标相等比较：行列都相同视为同一个格子
inline bool operator==(const Position& lhs, const Position& rhs) {
    return lhs.row == rhs.row && lhs.col == rhs.col;
}

/**
 * @brief 一次落子记录，用于 Replay 系统
 */
struct Move {
    int row;               ///< 落子行
    int col;               ///< 落子列
    Stone stone;           ///< 下了什么颜色
    std::string playerName; ///< 谁下的
};

// ========== 工具函数 ==========

/// 返回棋子的中文名："黑子" / "白子" / "空"
std::string stoneName(Stone stone);

/// 返回棋子的单字符符号：'X' / 'O' / '.'
char stoneSymbol(Stone stone);

/// 返回对手棋子颜色（Black ↔ White）
Stone oppositeStone(Stone stone);

} // namespace gomoku

#endif // GOMOKU_TYPES_H
