#include "Gomoku/UI.h"

#include <iostream>
#include <string>

namespace gomoku {

void UI::showWelcome() const {
    std::cout << "\n";
    std::cout << "============================================\n";
    std::cout << "              GOMOKU GAME SYSTEM            \n";
    std::cout << "============================================\n";
    std::cout << "1. Start Game\n";
    std::cout << "2. Game Rules\n";
    std::cout << "3. Control Help\n";
    std::cout << "4. Exit\n";
}

MainMenuChoice UI::askMainMenuChoice() const {
    std::cout << "Choose an option: ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "1") {
        return MainMenuChoice::StartGame;
    }
    if (input == "2") {
        return MainMenuChoice::Rules;
    }
    if (input == "3") {
        return MainMenuChoice::Controls;
    }
    if (input == "4") {
        return MainMenuChoice::Exit;
    }
    return MainMenuChoice::Invalid;
}

void UI::showRules() const {
    std::cout << "\nGame Rules\n";
    std::cout << "----------\n";
    std::cout << "1. The board size is 15 x 15.\n";
    std::cout << "2. Black moves first, then players move alternately.\n";
    std::cout << "3. A move must be placed on an empty position.\n";
    std::cout << "4. Five or more stones connected horizontally, vertically,\n";
    std::cout << "   or diagonally wins the game.\n";
    std::cout << "5. If the board is full and nobody wins, the game is a draw.\n";
}

void UI::showControls() const {
    std::cout << "\nControl Help\n";
    std::cout << "------------\n";
    std::cout << "Main menu: enter 1, 2, 3, or 4.\n";
    std::cout << "Move: enter row and column, for example: 8 8.\n";
    std::cout << "Undo: enter u during a game.\n";
    std::cout << "Resign and return: enter q during a game.\n";
    std::cout << "Replay: after a game ends, enter y when asked.\n";
    std::cout << "Coordinate range: 1 to 15.\n";
}

Player UI::askPlayer(const std::string& label, Stone stone) const {
    std::cout << label << " name (" << stoneName(stone) << " / " << stoneSymbol(stone) << "): ";

    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) {
        name = stoneName(stone) + " Player";
    }
    return Player(name, stone);
}

void UI::showBoard(const Board& board) const {
    std::cout << '\n' << board.render() << '\n';
}

void UI::showMessage(const std::string& message) const {
    std::cout << message << '\n';
}

void UI::waitForEnter() const {
    std::cout << "Press Enter to continue...";

    std::string ignored;
    std::getline(std::cin, ignored);
}

} // namespace gomoku
