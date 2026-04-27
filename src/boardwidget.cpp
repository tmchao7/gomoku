#include "boardwidget.h"

#include <QBrush>
#include <QFont>
#include <QLinearGradient>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QString>

#include <algorithm>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent) {
    setFixedSize(640, 680);
    setMouseTracking(true);
}

void BoardWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(238, 200, 128));
    drawBoard(painter);
    drawStones(painter);
    drawStatusText(painter);
}

void BoardWidget::mousePressEvent(QMouseEvent* event) {
    if (gameOver_ || event->button() != Qt::LeftButton) {
        return;
    }

    int row = 0;
    int col = 0;
    if (!pixelToGrid(event->pos(), row, col)) {
        return;
    }

    placeStone(row, col);
}

int BoardWidget::cellSize() const {
    const int usableWidth = width() - 2 * kMargin;
    return usableWidth / (kBoardSize - 1);
}

int BoardWidget::boardPixelSize() const {
    return cellSize() * (kBoardSize - 1);
}

QPoint BoardWidget::gridToPixel(int row, int col) const {
    return QPoint(kMargin + col * cellSize(), kMargin + row * cellSize());
}

bool BoardWidget::pixelToGrid(const QPoint& position, int& row, int& col) const {
    const int size = cellSize();
    const int boardEnd = kMargin + boardPixelSize();

    if (position.x() < kMargin - size / 2 || position.x() > boardEnd + size / 2 ||
        position.y() < kMargin - size / 2 || position.y() > boardEnd + size / 2) {
        return false;
    }

    col = (position.x() - kMargin + size / 2) / size;
    row = (position.y() - kMargin + size / 2) / size;
    col = std::clamp(col, 0, kBoardSize - 1);
    row = std::clamp(row, 0, kBoardSize - 1);

    const QPoint gridPoint = gridToPixel(row, col);
    const int clickTolerance = size / 2;
    return (position - gridPoint).manhattanLength() <= clickTolerance;
}

void BoardWidget::drawBoard(QPainter& painter) {
    const int size = cellSize();
    const int boardEnd = kMargin + boardPixelSize();

    painter.setPen(QPen(QColor(75, 45, 20), 2));
    for (int i = 0; i < kBoardSize; ++i) {
        const int offset = kMargin + i * size;
        painter.drawLine(kMargin, offset, boardEnd, offset);
        painter.drawLine(offset, kMargin, offset, boardEnd);
    }

    // 五子棋标准星位，方便玩家观察棋盘空间位置。
    painter.setBrush(QColor(75, 45, 20));
    painter.setPen(Qt::NoPen);
    const int starPoints[5][2] = {
        {3, 3},
        {3, 11},
        {7, 7},
        {11, 3},
        {11, 11},
    };
    for (const auto& point : starPoints) {
        painter.drawEllipse(gridToPixel(point[0], point[1]), 5, 5);
    }
}

void BoardWidget::drawStones(QPainter& painter) {
    const int radius = cellSize() / 2 - kStonePadding;

    for (int row = 0; row < kBoardSize; ++row) {
        for (int col = 0; col < kBoardSize; ++col) {
            const gomoku::Stone stone = board_.at(row, col);
            if (stone == gomoku::Stone::Empty) {
                continue;
            }

            const QPoint center = gridToPixel(row, col);
            const QRect stoneRect(center.x() - radius, center.y() - radius,
                                  radius * 2, radius * 2);

            if (stone == gomoku::Stone::Black) {
                QLinearGradient gradient(stoneRect.topLeft(), stoneRect.bottomRight());
                gradient.setColorAt(0.0, QColor(70, 70, 70));
                gradient.setColorAt(1.0, QColor(5, 5, 5));
                painter.setBrush(QBrush(gradient));
                painter.setPen(QPen(QColor(10, 10, 10), 1));
            } else {
                QLinearGradient gradient(stoneRect.topLeft(), stoneRect.bottomRight());
                gradient.setColorAt(0.0, QColor(255, 255, 255));
                gradient.setColorAt(1.0, QColor(210, 210, 210));
                painter.setBrush(QBrush(gradient));
                painter.setPen(QPen(QColor(120, 120, 120), 1));
            }

            painter.drawEllipse(stoneRect);
        }
    }
}

void BoardWidget::drawStatusText(QPainter& painter) {
    painter.setPen(QColor(45, 30, 18));
    painter.setFont(QFont("PingFang SC", 16, QFont::DemiBold));

    const QString player = currentStone_ == gomoku::Stone::Black ? "黑方" : "白方";
    const QString text = gameOver_ ? "游戏结束" : QString("当前回合：%1").arg(player);
    painter.drawText(QRect(0, height() - 52, width(), 32), Qt::AlignCenter, text);

    painter.setFont(QFont("PingFang SC", 11));
    painter.drawText(QRect(0, height() - 28, width(), 22),
                     Qt::AlignCenter,
                     "鼠标点击棋盘交叉点直接落子");
}

void BoardWidget::placeStone(int row, int col) {
    if (!board_.placeStone(row, col, currentStone_)) {
        return;
    }

    update();

    if (board_.hasFiveInRow(row, col)) {
        gameOver_ = true;
        showWinner(currentStone_);
        return;
    }

    switchPlayer();
}

void BoardWidget::switchPlayer() {
    currentStone_ = gomoku::oppositeStone(currentStone_);
    update();
}

void BoardWidget::showWinner(gomoku::Stone winner) {
    const QString winnerText = winner == gomoku::Stone::Black ? "黑方获胜" : "白方获胜";
    QMessageBox::information(this, "游戏结束", winnerText);
}
