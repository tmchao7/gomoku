#ifndef GOMOKU_REPLAY_H
#define GOMOKU_REPLAY_H

#include "Gomoku/Board.h"
#include "Gomoku/Types.h"

#include <vector>

namespace gomoku {

class Replay {
public:
    void clear();
    void addMove(const Move& move);
    bool undoLastMove();
    bool empty() const;
    int size() const;
    const std::vector<Move>& moves() const;
    std::vector<Board> snapshots() const;

private:
    std::vector<Move> moves_;
};

} // namespace gomoku

#endif // GOMOKU_REPLAY_H
