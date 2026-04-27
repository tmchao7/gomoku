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
    bool hasFiveInRow(int row, int col) const;
    std::vector<FiveLineCandidate> fiveLineCandidates(int row, int col) const;
    std::optional<FiveLineCandidate> findFiveLineByEndpoints(Position first,
                                                             Position second,
                                                             Stone stone) const;
    std::string render() const;
    std::string render(int cursorRow, int cursorCol) const;

private:
    std::array<std::array<Stone, kBoardSize>, kBoardSize> cells_;

    int countDirection(int row, int col, int deltaRow, int deltaCol, Stone stone) const;
};

} // namespace gomoku

#endif // GOMOKU_BOARD_H
