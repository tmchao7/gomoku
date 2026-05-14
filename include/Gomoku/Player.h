#ifndef GOMOKU_PLAYER_H
#define GOMOKU_PLAYER_H

#include "Gomoku/Types.h"

#include <string>

namespace gomoku {

/**
 * @class Player
 * @brief 玩家信息：姓名 + 执棋颜色。
 *
 * 用于落子记录中标识是哪位玩家下的这一步，也便于在界面上显示当前轮到谁。
 */
class Player {
public:
    /// 默认构造（空姓名，Empty）
    Player();

    /// 构造时指定玩家姓名和执棋颜色
    Player(std::string name, Stone stone);

    /// 获取玩家姓名
    const std::string& name() const;

    /// 获取玩家执棋颜色
    Stone stone() const;

private:
    std::string name_;  ///< 玩家姓名
    Stone stone_;       ///< 执棋颜色（Black 或 White）
};

} // namespace gomoku

#endif // GOMOKU_PLAYER_H
