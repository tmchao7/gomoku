#include "Gomoku/Board.h"
#include "Gomoku/Replay.h"

#include <algorithm>
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

bool containsPosition(const std::vector<gomoku::Position>& positions, int row, int col) {
    return std::any_of(positions.begin(), positions.end(), [row, col](const gomoku::Position& position) {
        return position.row == row && position.col == col;
    });
}

void testFiveLineCandidates() {
    gomoku::Board board;
    for (int col = 2; col <= 6; ++col) {
        require(board.placeStone(7, col, gomoku::Stone::Black), "place horizontal candidate stone");
    }
    for (int row = 5; row <= 9; ++row) {
        if (row != 7) {
            require(board.placeStone(row, 4, gomoku::Stone::Black), "place vertical candidate stone");
        }
    }

    const auto candidates = board.fiveLineCandidates(7, 4);
    require(candidates.size() == 2, "cross move should create two five-line candidates");
    require(candidates[0].positions.size() == 5, "candidate should contain exactly five stones");
    require(candidates[1].positions.size() == 5, "candidate should contain exactly five stones");

    int candidatesWithCenter = 0;
    for (const auto& candidate : candidates) {
        if (containsPosition(candidate.positions, 7, 4)) {
            ++candidatesWithCenter;
        }
    }
    require(candidatesWithCenter == 2, "all candidates should include the triggering move");
}

void testFindCandidateByEndpointsAndRemove() {
    gomoku::Board board;
    for (int col = 1; col <= 5; ++col) {
        require(board.placeStone(3, col, gomoku::Stone::White), "place removable line stone");
    }

    const auto candidate = board.findFiveLineByEndpoints({3, 1}, {3, 5}, gomoku::Stone::White);
    require(candidate.has_value(), "matching endpoints should identify a five-line candidate");
    require(candidate->positions.size() == 5, "endpoint candidate should contain five stones");

    board.removeStones(candidate->positions);
    for (int col = 1; col <= 5; ++col) {
        require(board.at(3, col) == gomoku::Stone::Empty, "removed line should become empty");
    }

    const auto invalid = board.findFiveLineByEndpoints({3, 1}, {3, 4}, gomoku::Stone::White);
    require(!invalid.has_value(), "four-stone endpoint span should be invalid");
}

void testReplaceStone() {
    gomoku::Board board;
    require(board.placeStone(4, 4, gomoku::Stone::White), "place stone to replace");
    require(board.replaceStone(4, 4, gomoku::Stone::Black), "replace occupied stone should succeed");
    require(board.at(4, 4) == gomoku::Stone::Black, "stone should be replaced by black");
    require(!board.replaceStone(0, 0, gomoku::Stone::Black), "replacing empty cell should fail");
    require(!board.replaceStone(4, 4, gomoku::Stone::Empty), "replacing with empty should fail");
}

} // namespace

int main() {
    testHorizontalWin();
    testVerticalWin();
    testDiagonalWins();
    testInvalidMovesAndUndo();
    testReplayRecordsAndUndo();
    testFiveLineCandidates();
    testFindCandidateByEndpointsAndRemove();
    testReplaceStone();

    std::cout << "All core tests passed.\n";
    return 0;
}
