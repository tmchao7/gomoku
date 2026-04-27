#include "Gomoku/Board.h"
#include "Gomoku/Replay.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "Test failed: " << message << '\n';
        std::exit(1);
    }
}

void testHorizontalWin() {
    gomoku::Board board;
    for (int col = 0; col < 5; ++col) {
        require(board.placeStone(7, col, gomoku::Stone::Black), "place horizontal stone");
    }
    require(board.hasFiveInRow(7, 4), "black should win horizontally");
}

void testVerticalWin() {
    gomoku::Board board;
    for (int row = 2; row < 7; ++row) {
        require(board.placeStone(row, 9, gomoku::Stone::White), "place vertical stone");
    }
    require(board.hasFiveInRow(6, 9), "white should win vertically");
}

void testDiagonalWins() {
    gomoku::Board downRight;
    for (int i = 0; i < 5; ++i) {
        require(downRight.placeStone(i, i, gomoku::Stone::Black), "place down-right diagonal stone");
    }
    require(downRight.hasFiveInRow(4, 4), "black should win on down-right diagonal");

    gomoku::Board upRight;
    for (int i = 0; i < 5; ++i) {
        require(upRight.placeStone(6 - i, i + 3, gomoku::Stone::White), "place up-right diagonal stone");
    }
    require(upRight.hasFiveInRow(2, 7), "white should win on up-right diagonal");
}

void testInvalidMovesAndUndo() {
    gomoku::Board board;
    require(!board.placeStone(-1, 0, gomoku::Stone::Black), "negative row should be invalid");
    require(!board.placeStone(0, 15, gomoku::Stone::Black), "column 15 should be invalid");
    require(board.placeStone(3, 3, gomoku::Stone::Black), "first move should succeed");
    require(!board.placeStone(3, 3, gomoku::Stone::White), "duplicate move should fail");
    require(board.removeStone(3, 3), "undo existing stone should succeed");
    require(board.at(3, 3) == gomoku::Stone::Empty, "cell should be empty after undo");
    require(!board.removeStone(3, 3), "undo empty cell should fail");
}

void testReplayRecordsAndUndo() {
    gomoku::Replay replay;
    replay.addMove({0, 0, gomoku::Stone::Black, "Alice"});
    replay.addMove({0, 1, gomoku::Stone::White, "Bob"});
    require(replay.size() == 2, "replay should contain two moves");
    require(replay.undoLastMove(), "replay undo should succeed");
    require(replay.size() == 1, "replay should contain one move after undo");

    const auto snapshots = replay.snapshots();
    require(snapshots.size() == 1, "one snapshot should be generated");
    require(snapshots[0].at(0, 0) == gomoku::Stone::Black, "snapshot should contain first move");
    require(snapshots[0].at(0, 1) == gomoku::Stone::Empty, "snapshot should not contain undone move");
}

} // namespace

int main() {
    testHorizontalWin();
    testVerticalWin();
    testDiagonalWins();
    testInvalidMovesAndUndo();
    testReplayRecordsAndUndo();

    std::cout << "All core tests passed.\n";
    return 0;
}
