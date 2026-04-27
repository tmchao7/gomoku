#ifndef GOMOKU_BOARD_H
#define GOMOKU_BOARD_H

#include "Gomoku/Types.h"

#include <array>
#include <string>
#include <vector>

namespace gomoku {

class Board {
public:
    Board();

    void reset();
    bool isInside(int row, int col) const;
    bool isEmpty(int row, int col) const;
    bool placeStone(int row, int col, Stone stone);
    bool removeStone(int row, int col);
    Stone at(int row, int col) const;
    bool isFull() const;
    bool hasFiveInRow(int row, int col) const;
    std::string render() const;
    std::string render(int cursorRow, int cursorCol) const;

private:
    std::array<std::array<Stone, kBoardSize>, kBoardSize> cells_;

    int countDirection(int row, int col, int deltaRow, int deltaCol, Stone stone) const;
};

} // namespace gomoku

#endif // GOMOKU_BOARD_H
