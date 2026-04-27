#ifndef GOMOKU_GAME_H
#define GOMOKU_GAME_H

#include "Gomoku/Board.h"
#include "Gomoku/Player.h"
#include "Gomoku/Replay.h"
#include "Gomoku/UI.h"

#include <array>

namespace gomoku {

class Game {
public:
    void run();

private:
    void startGame();
    void playReplay() const;
    bool handleUndo(int& currentPlayerIndex);

    Board board_;
    Replay replay_;
    UI ui_;
    std::array<Player, 2> players_;
};

} // namespace gomoku

#endif // GOMOKU_GAME_H
