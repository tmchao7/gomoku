#ifndef GOMOKU_BOARD_H
#define GOMOKU_BOARD_H

#include "Gomoku/Types.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace gomoku {

struct FiveLineCandidate {
    std::vector<Position> positions;
};

// Board 是纯规则层对象，不依赖 Qt。它只保存棋盘格子和围绕棋盘的规则判断，
// 因此可以被 GUI 程序和命令行测试程序共同复用。
class Board {
public:
    Board();

    void reset();
    bool isInside(int row, int col) const;
    bool isEmpty(int row, int col) const;
    bool placeStone(int row, int col, Stone stone);
    bool removeStone(int row, int col);
    void removeStones(const std::vector<Position>& positions);
    bool replaceStone(int row, int col, Stone stone);
    Stone at(int row, int col) const;
    bool isFull() const;

    // 只围绕最后落子点检查四个方向，不扫描整张棋盘。
    bool hasFiveInRow(int row, int col) const;

    // 进阶模式使用：找出所有“包含触发点”的连续五子候选，支持一步形成多条五连。
    std::vector<FiveLineCandidate> fiveLineCandidates(int row, int col) const;

    // 进阶模式使用：校验玩家点击的两个端点是否恰好构成一条连续五子。
    std::optional<FiveLineCandidate> findFiveLineByEndpoints(Position first,
                                                             Position second,
                                                             Stone stone) const;
    std::string render() const;
    std::string render(int cursorRow, int cursorCol) const;

private:
    // 棋盘唯一核心存储：15x15 固定数组，每个格子保存 Empty/Black/White。
    std::array<std::array<Stone, kBoardSize>, kBoardSize> cells_;

    int countDirection(int row, int col, int deltaRow, int deltaCol, Stone stone) const;
};

} // namespace gomoku

#endif // GOMOKU_BOARD_H
