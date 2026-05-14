#ifndef GOMOKU_BOARD_H
#define GOMOKU_BOARD_H

#include "Gomoku/Types.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

/**
 * @brief 一条由 5 个连续同色棋子构成的候选线（进阶模式使用）
 */
struct FiveLineCandidate {
    std::vector<Position> positions;  ///< 该线上的 5 个坐标，从左到右 / 从上到下排列
};

/**
 * @class Board
 * @brief 纯 C++ 棋盘逻辑，不依赖任何 GUI 框架。
 *
 * 核心职责：
 *   - 维护 15×15 的格子状态
 *   - 落子/移除/替换棋子
 *   - 五连判定（只检查最后落子点附近的四个方向，不扫描全盘）
 *
 * 该类的所有方法都是纯逻辑运算，可以被 GUI 和命令行测试共同复用。
 */
class Board {
public:
    Board();

    /// 重置棋盘为全部空白
    void reset();

    /// 判断 (row, col) 是否在棋盘范围内（0 ≤ row,col < 15）
    bool isInside(int row, int col) const;

    /// 判断 (row, col) 是否为空格
    bool isEmpty(int row, int col) const;

    /**
     * @brief 在 (row, col) 落下一枚棋子
     * @return true 落子成功；false 该位置已有棋子或超出棋盘
     */
    bool placeStone(int row, int col, Stone stone);

    /**
     * @brief 移除 (row, col) 的一枚棋子（普通模式悔棋用）
     * @return true 移除成功；false 该位置已是空格
     */
    bool removeStone(int row, int col);

    /// 批量移除多个位置的棋子（进阶模式消除五连用）
    void removeStones(const std::vector<Position>& positions);

    /**
     * @brief 将 (row, col) 的棋子替换为指定颜色（进阶模式转化用）
     * @note 拒绝替换空格，只允许将对方棋子变为己方棋子
     */
    bool replaceStone(int row, int col, Stone stone);

    /// 读取 (row, col) 的棋子状态
    Stone at(int row, int col) const;

    /// 棋盘是否已满（无空格）
    bool isFull() const;

    /**
     * @brief 检查以 (row, col) 为中心的四个方向上是否存在五连
     * @note 只检查经过该点的横/竖/两条斜线共四条线，不扫描全盘
     */
    bool hasFiveInRow(int row, int col) const;

    /**
     * @brief 找出所有包含 (row, col) 的连续五子候选线（进阶模式用）
     * @return 可能为空、一条或多条（例如十字交叉可同时形成两条五连）
     *
     * 实现方式：沿四个方向走到连续段的起点，然后滑动 5 格窗口进行检测。
     */
    std::vector<FiveLineCandidate> fiveLineCandidates(int row, int col) const;

    /**
     * @brief 校验玩家点击的两个端点是否恰好构成一条连续五子（进阶模式用）
     * @param first  玩家点击的第一个端点
     * @param second 玩家点击的第二个端点
     * @param stone  期望的棋子颜色
     * @return 若两点恰好相距 4 步且中间 5 格均为 stone，返回该五连信息；否则返回空
     */
    std::optional<FiveLineCandidate> findFiveLineByEndpoints(Position first,
                                                             Position second,
                                                             Stone stone) const;

    /// 将棋盘渲染为文本（用于调试/测试）
    std::string render() const;

    /// 在指定位置显示光标的文本渲染（用于测试/控制台）
    std::string render(int cursorRow, int cursorCol) const;

private:
    /// 核心数据：15×15 棋盘数组 cells_[row][col]
    std::array<std::array<Stone, kBoardSize>, kBoardSize> cells_;

    /**
     * @brief 从 (row, col) 出发，沿 (deltaRow, deltaCol) 方向的同色连续棋子数
     * @note 包括起点自身，用于辅助五连判定
     */
    int countDirection(int row, int col, int deltaRow, int deltaCol, Stone stone) const;
};

} // namespace gomoku

#endif // GOMOKU_BOARD_H
