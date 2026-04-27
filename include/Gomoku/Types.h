#ifndef GOMOKU_TYPES_H
#define GOMOKU_TYPES_H

#include <string>

namespace gomoku {

constexpr int kBoardSize = 15;

enum class Stone {
    Empty = 0,
    Black,
    White
};

enum class GameMode {
    Classic,
    AdvancedCapture
};

struct Position {
    int row;
    int col;
};

inline bool operator==(const Position& lhs, const Position& rhs) {
    return lhs.row == rhs.row && lhs.col == rhs.col;
}

struct Move {
    int row;
    int col;
    Stone stone;
    std::string playerName;
};

std::string stoneName(Stone stone);
char stoneSymbol(Stone stone);
Stone oppositeStone(Stone stone);

} // namespace gomoku

#endif // GOMOKU_TYPES_H
