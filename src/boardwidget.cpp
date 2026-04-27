#include "boardwidget.h"

#include <QBrush>
#include <QFont>
#include <QInputDialog>
#include <QLinearGradient>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QString>
#include <QTimer>

#include <algorithm>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent) {
    setFixedSize(640, 680);
    setMouseTracking(true);
    setupControls();

    // 等窗口进入事件循环后再弹出姓名输入框，启动体验更自然。
    QTimer::singleShot(0, this, [this]() {
        startNewGame();
    });
}

void BoardWidget::startNewGame() {
    askPlayerNames();

    board_.reset();
    replayBoard_.reset();
    replay_.clear();
    replaySnapshots_.clear();
    currentStone_ = gomoku::Stone::Black;
    gameOver_ = false;
    replayMode_ = false;
    replayStep_ = 0;
    updateControlButtons();
    update();
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
    if (gameOver_ || replayMode_ || event->button() != Qt::LeftButton) {
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

QString BoardWidget::currentPlayerName() const {
    return playerName(currentStone_);
}

QString BoardWidget::playerName(gomoku::Stone stone) const {
    if (stone == gomoku::Stone::Black) {
        return playerNames_[0];
    }
    if (stone == gomoku::Stone::White) {
        return playerNames_[1];
    }
    return "未知玩家";
}

QString BoardWidget::stoneLabel(gomoku::Stone stone) const {
    if (stone == gomoku::Stone::Black) {
        return "黑方";
    }
    if (stone == gomoku::Stone::White) {
        return "白方";
    }
    return "空位";
}

gomoku::Stone BoardWidget::displayedStoneAt(int row, int col) const {
    if (replayMode_) {
        return replayBoard_.at(row, col);
    }
    return board_.at(row, col);
}

void BoardWidget::setupControls() {
    undoButton_ = new QPushButton("悔棋", this);
    resignButton_ = new QPushButton("认输", this);
    replayButton_ = new QPushButton("复盘", this);
    restartButton_ = new QPushButton("重新开始", this);

    const int buttonWidth = 96;
    const int buttonHeight = 32;
    const int spacing = 12;
    const int totalWidth = buttonWidth * 4 + spacing * 3;
    int left = (width() - totalWidth) / 2;
    const int top = height() - 92;

    undoButton_->setGeometry(left, top, buttonWidth, buttonHeight);
    left += buttonWidth + spacing;
    resignButton_->setGeometry(left, top, buttonWidth, buttonHeight);
    left += buttonWidth + spacing;
    replayButton_->setGeometry(left, top, buttonWidth, buttonHeight);
    left += buttonWidth + spacing;
    restartButton_->setGeometry(left, top, buttonWidth, buttonHeight);

    connect(undoButton_, &QPushButton::clicked, this, [this]() {
        if (replayMode_) {
            showPreviousReplayStep();
        } else {
            undoLastMove();
        }
    });
    connect(resignButton_, &QPushButton::clicked, this, [this]() {
        if (replayMode_) {
            showNextReplayStep();
        } else {
            resignCurrentPlayer();
        }
    });
    connect(replayButton_, &QPushButton::clicked, this, [this]() {
        if (replayMode_) {
            exitReplayMode();
        } else {
            enterReplayMode();
        }
    });
    connect(restartButton_, &QPushButton::clicked, this, [this]() {
        startNewGame();
    });

    updateControlButtons();
}

void BoardWidget::updateControlButtons() {
    if (replayMode_) {
        undoButton_->setText("上一步");
        resignButton_->setText("下一步");
        replayButton_->setText("退出复盘");
        restartButton_->setText("重新开始");

        undoButton_->setEnabled(replayStep_ > 0);
        resignButton_->setEnabled(replayStep_ < replay_.size());
        replayButton_->setEnabled(true);
        restartButton_->setEnabled(true);
        return;
    }

    undoButton_->setText("悔棋");
    resignButton_->setText("认输");
    replayButton_->setText("复盘");
    restartButton_->setText("重新开始");

    undoButton_->setEnabled(!replay_.empty());
    resignButton_->setEnabled(!gameOver_);
    replayButton_->setEnabled(gameOver_ && !replay_.empty());
    restartButton_->setEnabled(true);
}

void BoardWidget::askPlayerNames() {
    bool ok = false;
    QString blackName = QInputDialog::getText(this,
                                              "玩家注册",
                                              "请输入黑方姓名：",
                                              QLineEdit::Normal,
                                              playerNames_[0],
                                              &ok);
    if (ok && !blackName.trimmed().isEmpty()) {
        playerNames_[0] = blackName.trimmed();
    }

    QString whiteName = QInputDialog::getText(this,
                                              "玩家注册",
                                              "请输入白方姓名：",
                                              QLineEdit::Normal,
                                              playerNames_[1],
                                              &ok);
    if (ok && !whiteName.trimmed().isEmpty()) {
        playerNames_[1] = whiteName.trimmed();
    }
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
            const gomoku::Stone stone = displayedStoneAt(row, col);
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

    QString text;
    if (replayMode_) {
        text = QString("复盘中：第 %1 / %2 手").arg(replayStep_).arg(replay_.size());
    } else {
        text = gameOver_
            ? "游戏结束"
            : QString("当前回合：%1：%2").arg(stoneLabel(currentStone_), currentPlayerName());
    }
    painter.drawText(QRect(0, height() - 52, width(), 28), Qt::AlignCenter, text);

    painter.setFont(QFont("PingFang SC", 11));
    painter.drawText(QRect(0, height() - 26, width(), 22),
                     Qt::AlignCenter,
                     replayMode_ ? "使用上一步/下一步查看历史落子"
                                 : "鼠标点击交叉点落子，可使用按钮悔棋、认输、复盘或重新开始");
}

void BoardWidget::placeStone(int row, int col) {
    if (!board_.placeStone(row, col, currentStone_)) {
        return;
    }
    replay_.addMove({row, col, currentStone_, currentPlayerName().toStdString()});

    updateControlButtons();
    update();

    if (board_.hasFiveInRow(row, col)) {
        gameOver_ = true;
        updateControlButtons();
        showWinner(currentStone_);
        return;
    }

    switchPlayer();
}

void BoardWidget::switchPlayer() {
    currentStone_ = gomoku::oppositeStone(currentStone_);
    updateControlButtons();
    update();
}

void BoardWidget::undoLastMove() {
    if (replayMode_) {
        return;
    }

    if (replay_.empty()) {
        QMessageBox::information(this, "无法悔棋", "当前还没有可以撤销的落子。");
        return;
    }

    const gomoku::Move lastMove = replay_.moves().back();
    if (!board_.removeStone(lastMove.row, lastMove.col)) {
        QMessageBox::warning(this, "悔棋失败", "棋盘状态异常，无法撤销最后一步。");
        return;
    }

    replay_.undoLastMove();
    currentStone_ = lastMove.stone;
    gameOver_ = false;
    updateControlButtons();
    update();
}

void BoardWidget::resignCurrentPlayer() {
    if (gameOver_ || replayMode_) {
        return;
    }

    const gomoku::Stone loser = currentStone_;
    const gomoku::Stone winner = gomoku::oppositeStone(loser);
    gameOver_ = true;
    updateControlButtons();
    update();

    const QString message = QString("%1：%2 认输。\n%3：%4 获胜！")
        .arg(stoneLabel(loser), playerName(loser), stoneLabel(winner), playerName(winner));
    QMessageBox::information(this, "游戏结束", message);

    const QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        "复盘",
        "是否立即进入复盘？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    if (choice == QMessageBox::Yes) {
        enterReplayMode();
    }
}

void BoardWidget::enterReplayMode() {
    if (replay_.empty()) {
        QMessageBox::information(this, "无法复盘", "当前还没有可以复盘的落子。");
        return;
    }

    replaySnapshots_ = replay_.snapshots();
    replayMode_ = true;
    replayStep_ = 0;
    applyReplayStep();
}

void BoardWidget::exitReplayMode() {
    replayMode_ = false;
    replayStep_ = 0;
    replayBoard_.reset();
    updateControlButtons();
    update();
}

void BoardWidget::showPreviousReplayStep() {
    if (!replayMode_ || replayStep_ <= 0) {
        return;
    }

    --replayStep_;
    applyReplayStep();
}

void BoardWidget::showNextReplayStep() {
    if (!replayMode_ || replayStep_ >= replay_.size()) {
        return;
    }

    ++replayStep_;
    applyReplayStep();
}

void BoardWidget::applyReplayStep() {
    replayBoard_.reset();
    if (replayStep_ > 0 && replayStep_ <= static_cast<int>(replaySnapshots_.size())) {
        replayBoard_ = replaySnapshots_[static_cast<std::size_t>(replayStep_ - 1)];
    }

    updateControlButtons();
    update();
}

void BoardWidget::showWinner(gomoku::Stone winner) {
    const QString winnerText = QString("%1：%2 获胜！")
        .arg(stoneLabel(winner), playerName(winner));
    QMessageBox::information(this, "游戏结束", winnerText);

    const QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        "复盘",
        "是否立即进入复盘？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
    if (choice == QMessageBox::Yes) {
        enterReplayMode();
    }
}
