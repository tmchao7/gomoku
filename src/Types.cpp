#include "Gomoku/Types.h"

namespace gomoku {

std::string stoneName(Stone stone) {
    switch (stone) {
    case Stone::Black:
        return "Black";
    case Stone::White:
        return "White";
    case Stone::Empty:
    default:
        return "Empty";
    }
}

char stoneSymbol(Stone stone) {
    switch (stone) {
    case Stone::Black:
        return 'X';
    case Stone::White:
        return 'O';
    case Stone::Empty:
    default:
        return '.';
    }
}

Stone oppositeStone(Stone stone) {
    if (stone == Stone::Black) {
        return Stone::White;
    }
    if (stone == Stone::White) {
        return Stone::Black;
    }
    return Stone::Empty;
}

} // namespace gomoku
