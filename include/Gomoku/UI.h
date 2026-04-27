#ifndef GOMOKU_UI_H
#define GOMOKU_UI_H

#include "Gomoku/Board.h"
#include "Gomoku/Player.h"

#include <string>

namespace gomoku {

enum class MainMenuChoice {
    StartGame,
    Rules,
    Controls,
    Exit,
    Invalid
};

class UI {
public:
    void showWelcome() const;
    MainMenuChoice askMainMenuChoice() const;
    void showRules() const;
    void showControls() const;
    Player askPlayer(const std::string& label, Stone stone) const;
    void showBoard(const Board& board) const;
    void showMessage(const std::string& message) const;
    void waitForEnter() const;
};

} // namespace gomoku

#endif // GOMOKU_UI_H
