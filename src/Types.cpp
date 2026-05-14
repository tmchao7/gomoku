#include "Gomoku/Types.h"

namespace gomoku {

/// 棋子中文名，用于界面显示
std::string stoneName(Stone stone) {
    switch (stone) {
    case Stone::Black:
        return "黑方";
    case Stone::White:
        return "白方";
    case Stone::Empty:
    default:
        return "空位";
    }
}

/// 棋子符号，用于控制台文本渲染
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

/// 返回对方棋子颜色：黑 ↔ 白，空则返回空
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
