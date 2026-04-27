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
