#include "Gomoku/Board.h"

#include <iomanip>
#include <sstream>

namespace gomoku {

Board::Board() {
    reset();
}

void Board::reset() {
    for (auto& row : cells_) {
        row.fill(Stone::Empty);
    }
}

bool Board::isInside(int row, int col) const {
    return row >= 0 && row < kBoardSize && col >= 0 && col < kBoardSize;
}

bool Board::isEmpty(int row, int col) const {
    return isInside(row, col) && cells_[row][col] == Stone::Empty;
}

bool Board::placeStone(int row, int col, Stone stone) {
    if (stone == Stone::Empty || !isEmpty(row, col)) {
        return false;
    }
    cells_[row][col] = stone;
    return true;
}

bool Board::removeStone(int row, int col) {
    if (!isInside(row, col) || cells_[row][col] == Stone::Empty) {
        return false;
    }
    cells_[row][col] = Stone::Empty;
    return true;
}

Stone Board::at(int row, int col) const {
    if (!isInside(row, col)) {
        return Stone::Empty;
    }
    return cells_[row][col];
}

bool Board::isFull() const {
    for (const auto& row : cells_) {
        for (Stone cell : row) {
            if (cell == Stone::Empty) {
                return false;
            }
        }
    }
    return true;
}

bool Board::hasFiveInRow(int row, int col) const {
    if (!isInside(row, col)) {
        return false;
    }

    const Stone stone = cells_[row][col];
    if (stone == Stone::Empty) {
        return false;
    }

    const int directions[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1},
    };

    for (const auto& direction : directions) {
        const int count = 1
            + countDirection(row, col, direction[0], direction[1], stone)
            + countDirection(row, col, -direction[0], -direction[1], stone);
        if (count >= 5) {
            return true;
        }
    }

    return false;
}

std::string Board::render() const {
    return render(-1, -1);
}

std::string Board::render(int cursorRow, int cursorCol) const {
    std::ostringstream out;
    out << "    ";
    for (int col = 1; col <= kBoardSize; ++col) {
        out << std::setw(2) << col << ' ';
    }
    out << '\n';

    for (int row = 0; row < kBoardSize; ++row) {
        out << std::setw(2) << row + 1 << "  ";
        for (int col = 0; col < kBoardSize; ++col) {
            const char symbol = stoneSymbol(cells_[row][col]);
            if (row == cursorRow && col == cursorCol) {
                out << '[' << symbol << ']';
            } else {
                out << ' ' << symbol << ' ';
            }
        }
        out << '\n';
    }
    return out.str();
}

int Board::countDirection(int row, int col, int deltaRow, int deltaCol, Stone stone) const {
    int count = 0;
    int currentRow = row + deltaRow;
    int currentCol = col + deltaCol;

    while (isInside(currentRow, currentCol) && cells_[currentRow][currentCol] == stone) {
        ++count;
        currentRow += deltaRow;
        currentCol += deltaCol;
    }

    return count;
}

} // namespace gomoku
