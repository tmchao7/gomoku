#include "Gomoku/Board.h"

#include <cstddef>
#include <cstdlib>
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

void Board::removeStones(const std::vector<Position>& positions) {
    for (const Position& position : positions) {
        if (isInside(position.row, position.col)) {
            cells_[position.row][position.col] = Stone::Empty;
        }
    }
}

bool Board::replaceStone(int row, int col, Stone stone) {
    if (stone == Stone::Empty || !isInside(row, col) || cells_[row][col] == Stone::Empty) {
        return false;
    }

    cells_[row][col] = stone;
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

    // 四个方向：水平、垂直、主对角线、副对角线。每个方向以落子为中心向正反两侧计数。
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

std::vector<FiveLineCandidate> Board::fiveLineCandidates(int row, int col) const {
    std::vector<FiveLineCandidate> candidates;
    if (!isInside(row, col)) {
        return candidates;
    }

    const Stone stone = cells_[row][col];
    if (stone == Stone::Empty) {
        return candidates;
    }

    const int directions[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1},
    };

    for (const auto& direction : directions) {
        // 先退到该方向连续同色棋子的起点，再沿方向滑动 5 子窗口收集所有候选
        std::vector<Position> line;
        int currentRow = row;
        int currentCol = col;
        while (isInside(currentRow - direction[0], currentCol - direction[1]) &&
               cells_[currentRow - direction[0]][currentCol - direction[1]] == stone) {
            currentRow -= direction[0];
            currentCol -= direction[1];
        }

        while (isInside(currentRow, currentCol) && cells_[currentRow][currentCol] == stone) {
            line.push_back({currentRow, currentCol});
            currentRow += direction[0];
            currentCol += direction[1];
        }

        if (line.size() < 5) {
            continue;
        }

        // 滑动窗口：对长度 ≥5 的连续线段，保留包含触发位置的 5 子窗口
        for (std::size_t start = 0; start + 5 <= line.size(); ++start) {
            FiveLineCandidate candidate;
            candidate.positions.assign(line.begin() + static_cast<std::ptrdiff_t>(start),
                                       line.begin() + static_cast<std::ptrdiff_t>(start + 5));
            for (const Position& position : candidate.positions) {
                if (position.row == row && position.col == col) {
                    candidates.push_back(candidate);
                    break;
                }
            }
        }
    }

    return candidates;
}

std::optional<FiveLineCandidate> Board::findFiveLineByEndpoints(Position first,
                                                                 Position second,
                                                                 Stone stone) const {
    if (stone == Stone::Empty || !isInside(first.row, first.col) || !isInside(second.row, second.col)) {
        return std::nullopt;
    }

    const int deltaRow = second.row - first.row;
    const int deltaCol = second.col - first.col;
    const int stepRow = (deltaRow == 0) ? 0 : (deltaRow > 0 ? 1 : -1);
    const int stepCol = (deltaCol == 0) ? 0 : (deltaCol > 0 ? 1 : -1);

    // 两个端点必须恰好相距 4 格（共 5 颗棋子），且方向为水平/垂直/对角线。
    // 这里先校验几何关系，再逐格检查棋子颜色，避免把非连续五子误判为可消除线段。
    const int absDR = std::abs(deltaRow);
    const int absDC = std::abs(deltaCol);
    const bool isStraightLine = (deltaRow == 0 || deltaCol == 0 || absDR == absDC);
    const bool isFiveLong = std::max(absDR, absDC) == 4;
    if (!isStraightLine || !isFiveLong || (stepRow == 0 && stepCol == 0)) {
        return std::nullopt;
    }

    FiveLineCandidate candidate;
    for (int i = 0; i < 5; ++i) {
        const int currentRow = first.row + stepRow * i;
        const int currentCol = first.col + stepCol * i;
        if (!isInside(currentRow, currentCol) || cells_[currentRow][currentCol] != stone) {
            return std::nullopt;
        }
        candidate.positions.push_back({currentRow, currentCol});
    }

    return candidate;
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
