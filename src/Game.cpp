#include "Gomoku/Game.h"

#include <iostream>
#include <sstream>
#include <string>

namespace gomoku {
namespace {

bool parseMoveInput(const std::string& input, int& row, int& col) {
    std::istringstream stream(input);
    int humanRow = 0;
    int humanCol = 0;
    if (!(stream >> humanRow >> humanCol)) {
        return false;
    }

    std::string extra;
    if (stream >> extra) {
        return false;
    }

    row = humanRow - 1;
    col = humanCol - 1;
    return true;
}

bool isYes(const std::string& input) {
    return input == "y" || input == "Y";
}

bool isUndo(const std::string& input) {
    return input == "u" || input == "U";
}

bool isQuit(const std::string& input) {
    return input == "q" || input == "Q";
}

} // namespace

void Game::run() {
    bool running = true;
    while (running) {
        ui_.showWelcome();

        switch (ui_.askMainMenuChoice()) {
        case MainMenuChoice::StartGame:
            startGame();
            break;
        case MainMenuChoice::Rules:
            ui_.showRules();
            ui_.waitForEnter();
            break;
        case MainMenuChoice::Controls:
            ui_.showControls();
            ui_.waitForEnter();
            break;
        case MainMenuChoice::Exit:
            ui_.showMessage("Goodbye.");
            running = false;
            break;
        case MainMenuChoice::Invalid:
        default:
            ui_.showMessage("Invalid menu option. Please try again.");
            break;
        }
    }
}

void Game::startGame() {
    board_.reset();
    replay_.clear();

    ui_.showMessage("\nLogin");
    players_[0] = ui_.askPlayer("Black player", Stone::Black);
    players_[1] = ui_.askPlayer("White player", Stone::White);

    int currentPlayerIndex = 0;
    const Player* winner = nullptr;
    bool draw = false;

    while (winner == nullptr && !draw) {
        ui_.showBoard(board_);

        const Player& currentPlayer = players_[currentPlayerIndex];
        std::cout << currentPlayer.name() << " (" << stoneName(currentPlayer.stone())
                  << " / " << stoneSymbol(currentPlayer.stone())
                  << ") move [row col], u=undo, q=resign: ";

        std::string input;
        std::getline(std::cin, input);

        if (isUndo(input)) {
            if (!handleUndo(currentPlayerIndex)) {
                ui_.showMessage("No move can be undone.");
            }
            continue;
        }

        if (isQuit(input)) {
            winner = &players_[1 - currentPlayerIndex];
            ui_.showMessage(currentPlayer.name() + " resigned.");
            break;
        }

        int row = 0;
        int col = 0;
        if (!parseMoveInput(input, row, col)) {
            ui_.showMessage("Invalid input. Please enter two numbers, such as: 8 8.");
            continue;
        }

        if (!board_.placeStone(row, col, currentPlayer.stone())) {
            ui_.showMessage("Invalid move. The position is out of range or already occupied.");
            continue;
        }

        replay_.addMove({row, col, currentPlayer.stone(), currentPlayer.name()});

        if (board_.hasFiveInRow(row, col)) {
            winner = &currentPlayer;
            break;
        }

        if (board_.isFull()) {
            draw = true;
            break;
        }

        currentPlayerIndex = 1 - currentPlayerIndex;
    }

    ui_.showBoard(board_);
    if (winner != nullptr) {
        ui_.showMessage("Winner: " + winner->name() + " (" + stoneName(winner->stone()) + ")");
        ui_.showMessage("Total moves: " + std::to_string(replay_.size()));
    } else if (draw) {
        ui_.showMessage("The game is a draw.");
        ui_.showMessage("Total moves: " + std::to_string(replay_.size()));
    }

    std::cout << "Watch replay? (y/n): ";
    std::string replayChoice;
    std::getline(std::cin, replayChoice);
    if (isYes(replayChoice)) {
        playReplay();
    }

    ui_.waitForEnter();
}

void Game::playReplay() const {
    if (replay_.empty()) {
        ui_.showMessage("No replay data.");
        return;
    }

    ui_.showMessage("\nReplay starts.");
    Board replayBoard;

    int step = 1;
    for (const Move& move : replay_.moves()) {
        replayBoard.placeStone(move.row, move.col, move.stone);
        ui_.showBoard(replayBoard);

        std::cout << "Step " << step << ": " << move.playerName
                  << " placed " << stoneName(move.stone)
                  << " at (" << move.row + 1 << ", " << move.col + 1 << ").\n";
        ++step;

        ui_.waitForEnter();
    }
    ui_.showMessage("Replay finished.");
}

bool Game::handleUndo(int& currentPlayerIndex) {
    if (replay_.empty()) {
        return false;
    }

    const Move lastMove = replay_.moves().back();
    if (!board_.removeStone(lastMove.row, lastMove.col)) {
        return false;
    }

    replay_.undoLastMove();
    currentPlayerIndex = (lastMove.stone == players_[0].stone()) ? 0 : 1;
    return true;
}

} // namespace gomoku
