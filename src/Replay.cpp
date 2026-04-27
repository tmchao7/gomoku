#include "Gomoku/Replay.h"

namespace gomoku {

void Replay::clear() {
    moves_.clear();
}

void Replay::addMove(const Move& move) {
    moves_.push_back(move);
}

bool Replay::undoLastMove() {
    if (moves_.empty()) {
        return false;
    }
    moves_.pop_back();
    return true;
}

bool Replay::empty() const {
    return moves_.empty();
}

int Replay::size() const {
    return static_cast<int>(moves_.size());
}

const std::vector<Move>& Replay::moves() const {
    return moves_;
}

std::vector<Board> Replay::snapshots() const {
    std::vector<Board> result;
    Board board;

    for (const Move& move : moves_) {
        board.placeStone(move.row, move.col, move.stone);
        result.push_back(board);
    }

    return result;
}

} // namespace gomoku
