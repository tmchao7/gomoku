#include "Gomoku/Board.h"

#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace gomoku {

/// 构造函数：初始化棋盘为全空
Board::Board() {
    reset();
}

/// 重置棋盘：所有格子设为 Empty
void Board::reset() {
    for (auto& row : cells_) {
        row.fill(Stone::Empty);
    }
}

/// 判断坐标是否在 15×15 棋盘范围内
bool Board::isInside(int row, int col) const {
    return row >= 0 && row < kBoardSize && col >= 0 && col < kBoardSize;
}

/// 判断指定位置是否为空（同时检查越界）
bool Board::isEmpty(int row, int col) const {
    return isInside(row, col) && cells_[row][col] == Stone::Empty;
}

/**
 * @brief 在指定位置落下一枚棋子
 *
 * 先检查该位置是否为空，再检查 stone 是否为有效颜色（拒绝 Empty），
 * 全部通过才写入棋盘。
 * @return true 落子成功；false 位置已有棋子、超出棋盘或 stone 为 Empty
 */
bool Board::placeStone(int row, int col, Stone stone) {
    if (stone == Stone::Empty || !isEmpty(row, col)) {
        return false;
    }
    cells_[row][col] = stone;
    return true;
}

/**
 * @brief 移除指定位置的一枚棋子（普通模式悔棋用）
 * @return true 移除成功；false 该位置已是空格或越界
 */
bool Board::removeStone(int row, int col) {
    if (!isInside(row, col) || cells_[row][col] == Stone::Empty) {
        return false;
    }
    cells_[row][col] = Stone::Empty;
    return true;
}

/// 批量移除多个位置的棋子（进阶模式消除五连时调用），越界位置自动跳过
void Board::removeStones(const std::vector<Position>& positions) {
    for (const Position& position : positions) {
        if (isInside(position.row, position.col)) {
            cells_[position.row][position.col] = Stone::Empty;
        }
    }
}

/**
 * @brief 将指定位置的棋子替换为另一种颜色（进阶模式转化用）
 *
 * 用于将对方棋子转化为己方棋子。拒绝替换空格（必须已有棋子），
 * 也拒绝传入 Empty 作为目标颜色。
 * @return true 替换成功；false 参数非法
 */
bool Board::replaceStone(int row, int col, Stone stone) {
    if (stone == Stone::Empty || !isInside(row, col) || cells_[row][col] == Stone::Empty) {
        return false;
    }

    cells_[row][col] = stone;
    return true;
}

/// 读取指定位置的棋子状态，越界返回 Empty
Stone Board::at(int row, int col) const {
    if (!isInside(row, col)) {
        return Stone::Empty;
    }
    return cells_[row][col];
}

/// 遍历全盘，判断是否所有格子都已被占据
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

/**
 * @brief 以 (row, col) 为中心，检查四个方向上是否有五连
 *
 * 实现思路：
 * 1. 获取该位置的棋子颜色，如果是空格则直接返回 false。
 * 2. 以该位置为起点，分别沿水平、垂直、两条对角线四个方向，
 *    向正反两侧统计连续同色棋子数，正反合计 + 自身 ≥ 5 即判定为五连。
 * 3. 不扫描全盘，只检查经过落子点的四条线，效率 O(1)。
 *
 * @return true 存在五连；false 不存在或该位置为空格
 */
bool Board::hasFiveInRow(int row, int col) const {
    if (!isInside(row, col)) {
        return false;
    }

    const Stone stone = cells_[row][col];
    if (stone == Stone::Empty) {
        return false;
    }

    // 四个方向向量：{deltaRow, deltaCol}
    // 分别对应：水平、垂直、主对角线(↘)、副对角线(↗)
    const int directions[4][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1},
    };

    for (const auto& direction : directions) {
        // 沿正方向（如向右）计数 + 沿反方向（如向左）计数 + 自身
        const int count = 1
            + countDirection(row, col, direction[0], direction[1], stone)
            + countDirection(row, col, -direction[0], -direction[1], stone);
        if (count >= 5) {
            return true;
        }
    }

    return false;
}

/**
 * @brief 找出所有经过 (row, col) 的连续五子候选线（进阶模式用）
 *
 * 和 hasFiveInRow 类似地遍历四个方向，但更进一步：
 * 1. 沿方向退到连续同色棋子的起点。
 * 2. 从起点向前走，收集整个连续线段的所有坐标。
 * 3. 如果线段长度 ≥ 5，用滑动窗口方式提取所有长度为 5、且包含 (row, col) 的候选。
 *
 * 例如，一行有 7 颗同色棋子，触发点在第 4 颗，会返回 3 个候选：
 *   [位置1~5]、[位置2~6]、[位置3~7] 都包含触发点。
 * 十字交叉的情况则可能从两个方向各返回多个候选。
 *
 * @return 所有符合条件的五连候选，可能为空
 */
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
        // 第一步：退到连续段的起点（沿反方向走到第一个不同色或出界）
        std::vector<Position> line;
        int currentRow = row;
        int currentCol = col;
        while (isInside(currentRow - direction[0], currentCol - direction[1]) &&
               cells_[currentRow - direction[0]][currentCol - direction[1]] == stone) {
            currentRow -= direction[0];
            currentCol -= direction[1];
        }

        // 第二步：从起点开始沿正方向收集整个连续段
        while (isInside(currentRow, currentCol) && cells_[currentRow][currentCol] == stone) {
            line.push_back({currentRow, currentCol});
            currentRow += direction[0];
            currentCol += direction[1];
        }

        // 少于 5 颗不可能构成五连，跳过
        if (line.size() < 5) {
            continue;
        }

        // 第三步：滑动窗口，保留包含触发位置 (row, col) 的所有五连
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

/**
 * @brief 校验玩家点击的两个端点是否构成一条五连（进阶模式用）
 *
 * 玩家在界面上点了两个端点，函数需要验证：
 * 1. 两点在水平/垂直/对角线方向上正好相距 4 步（即中间包含 5 个格子）。
 * 2. 中间 5 个格子的棋子和 players 指定的 stone 颜色一致。
 *
 * 先做几何校验（方向、长度），再逐格检查颜色，避免误判。
 *
 * @return 如果合法，返回该五连的 5 个坐标；否则返回空
 */
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

    // 几何校验：两点必须构成水平/垂直/对角线，且距离为 4
    const int absDR = std::abs(deltaRow);
    const int absDC = std::abs(deltaCol);
    const bool isStraightLine = (deltaRow == 0 || deltaCol == 0 || absDR == absDC);
    const bool isFiveLong = std::max(absDR, absDC) == 4;
    if (!isStraightLine || !isFiveLong || (stepRow == 0 && stepCol == 0)) {
        return std::nullopt;
    }

    // 从 first 出发，沿方向走 5 步，逐格检查颜色
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

/// 无光标版本的渲染委托给有光标版本，传入 (-1,-1) 表示不显示光标
std::string Board::render() const {
    return render(-1, -1);
}

/**
 * @brief 将棋盘格式化为文本（用于调试/命令行测试）
 *
 * 输出包含列号标题行和行号前缀，示例：
 *     1  2  3  4  5 ...
 *   1  .  .  .  .  .
 *   2  . [X] .  .  .
 *   3  .  .  O  .  .
 *
 * cursorRow/cursorCol 不为 -1 时，对应格子用方括号高亮。
 */
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

/**
 * @brief 统计从 (row, col) 出发，沿 (deltaRow, deltaCol) 方向的连续同色棋子数
 *
 * 从 (row + deltaRow, col + deltaCol) 开始检查，不包括起点自身。
 * hasFiveInRow 用它来分别计算正反两个方向的长度。
 *
 * 例如在水平方向 countDirection(7,7, 0,1, Black) 会从 (7,8) 开始向右数
 * 连续黑子，返回连续的黑子个数。
 */
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
