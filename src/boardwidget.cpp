#include "boardwidget.h"

#include <QBrush>
#include <QFont>
#include <QInputDialog>
#include <QLinearGradient>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <algorithm>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent) {
    setFixedSize(640, 680);
    setMouseTracking(true);
    setupControls();
    autoReplayTimer_ = new QTimer(this);
    autoReplayTimer_->setInterval(700);
    connect(autoReplayTimer_, &QTimer::timeout, this, [this]() {
        advanceAutoReplay();
    });

    // 等窗口进入事件循环后再弹出姓名输入框，启动体验更自然。
    QTimer::singleShot(0, this, [this]() {
        startNewGame();
    });
}

void BoardWidget::startNewGame() {
    askPlayerNames();
    askGameMode();

    board_.reset();
    replayBoard_.reset();
    replay_.clear();
    replaySnapshots_.clear();
    currentStone_ = gomoku::Stone::Black;
    scores_ = {0, 0};
    state_ = InteractionState::Playing;
    gameOver_ = false;
    replayMode_ = false;
    replayPlaybackMode_ = ReplayPlaybackMode::Manual;
    replayStep_ = 0;
    stopAutoReplay();
    history_.clear();
    selectableLines_.clear();
    selectedEndpoints_.clear();
    saveSnapshot("初始棋盘");
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
    drawSelectableLines(painter);
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

    if (gameMode_ == gomoku::GameMode::AdvancedCapture) {
        handleAdvancedClick(row, col);
    } else {
        placeStone(row, col);
    }
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

int BoardWidget::playerIndex(gomoku::Stone stone) const {
    return stone == gomoku::Stone::Black ? 0 : 1;
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

QString BoardWidget::modeLabel() const {
    return gameMode_ == gomoku::GameMode::Classic ? "普通模式" : "进阶模式";
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
            if (replayPlaybackMode_ == ReplayPlaybackMode::Auto) {
                toggleAutoReplay();
            } else {
                showPreviousReplayStep();
            }
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
        const int maxStep = gameMode_ == gomoku::GameMode::AdvancedCapture
            ? static_cast<int>(replaySnapshots_.size())
            : replay_.size();
        if (replayPlaybackMode_ == ReplayPlaybackMode::Auto) {
            undoButton_->setText(autoReplayTimer_->isActive() ? "暂停" : "继续");
            resignButton_->setText("下一步");
            undoButton_->setEnabled(replayStep_ < maxStep);
            resignButton_->setEnabled(!autoReplayTimer_->isActive() && replayStep_ < maxStep);
        } else {
            undoButton_->setText("上一步");
            resignButton_->setText("下一步");
            undoButton_->setEnabled(replayStep_ > 0);
            resignButton_->setEnabled(replayStep_ < maxStep);
        }
        replayButton_->setText("退出复盘");
        restartButton_->setText("重新开始");

        replayButton_->setEnabled(true);
        restartButton_->setEnabled(true);
        return;
    }

    undoButton_->setText("悔棋");
    resignButton_->setText("认输");
    replayButton_->setText("复盘");
    restartButton_->setText("重新开始");

    const bool hasAdvancedHistory = gameMode_ == gomoku::GameMode::AdvancedCapture && history_.size() > 1;
    undoButton_->setEnabled(gameMode_ == gomoku::GameMode::AdvancedCapture ? hasAdvancedHistory : !replay_.empty());
    resignButton_->setEnabled(!gameOver_);
    replayButton_->setEnabled(gameOver_ && (gameMode_ == gomoku::GameMode::AdvancedCapture ? hasAdvancedHistory : !replay_.empty()));
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

void BoardWidget::askGameMode() {
    const QStringList modes = {"普通模式", "进阶模式"};
    bool ok = false;
    const QString choice = QInputDialog::getItem(this,
                                                "模式选择",
                                                "请选择游戏模式：",
                                                modes,
                                                gameMode_ == gomoku::GameMode::Classic ? 0 : 1,
                                                false,
                                                &ok);
    if (ok && choice == "进阶模式") {
        gameMode_ = gomoku::GameMode::AdvancedCapture;
    } else {
        gameMode_ = gomoku::GameMode::Classic;
    }
}

BoardWidget::ReplayPlaybackMode BoardWidget::askReplayPlaybackMode() {
    const QStringList modes = {"自动复盘", "手动复盘"};
    bool ok = false;
    const QString choice = QInputDialog::getItem(this,
                                                "复盘模式",
                                                "请选择复盘方式：",
                                                modes,
                                                0,
                                                false,
                                                &ok);
    if (ok && choice == "自动复盘") {
        return ReplayPlaybackMode::Auto;
    }
    return ReplayPlaybackMode::Manual;
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

void BoardWidget::drawSelectableLines(QPainter& painter) {
    if (!replayMode_ && state_ != InteractionState::SelectingLine) {
        return;
    }

    const QColor colors[] = {
        QColor(210, 40, 40),
        QColor(30, 110, 220),
        QColor(30, 150, 70),
        QColor(160, 80, 210),
        QColor(230, 130, 20),
        QColor(0, 150, 160),
    };

    int index = 0;
    for (const gomoku::FiveLineCandidate& candidate : selectableLines_) {
        if (candidate.positions.empty()) {
            continue;
        }

        painter.setPen(QPen(colors[index % 6], 4, Qt::SolidLine, Qt::RoundCap));
        const gomoku::Position first = candidate.positions.front();
        const gomoku::Position last = candidate.positions.back();
        painter.drawLine(gridToPixel(first.row, first.col), gridToPixel(last.row, last.col));

        painter.setBrush(colors[index % 6]);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(gridToPixel(first.row, first.col), 8, 8);
        painter.drawEllipse(gridToPixel(last.row, last.col), 8, 8);
        ++index;
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
        const int maxStep = gameMode_ == gomoku::GameMode::AdvancedCapture
            ? static_cast<int>(replaySnapshots_.size())
            : replay_.size();
        if (gameMode_ == gomoku::GameMode::AdvancedCapture) {
            std::array<int, 2> replayScores = {0, 0};
            if (replayStep_ > 0 && replayStep_ < static_cast<int>(history_.size())) {
                replayScores = history_[static_cast<std::size_t>(replayStep_)].scores;
            }
            text = QString("复盘中：第 %1 / %2 步｜比分 黑 %3 : 白 %4")
                       .arg(replayStep_)
                       .arg(maxStep)
                       .arg(replayScores[0])
                       .arg(replayScores[1]);
        } else {
            text = QString("复盘中：第 %1 / %2 步").arg(replayStep_).arg(maxStep);
        }
    } else if (gameMode_ == gomoku::GameMode::AdvancedCapture) {
        if (state_ == InteractionState::SelectingLine) {
            text = QString("%1：请选择要消除五连的两个端点").arg(modeLabel());
        } else if (state_ == InteractionState::ConvertingStone) {
            text = QString("%1：%2：%3 请选择一颗对方棋子转化")
                       .arg(modeLabel(), stoneLabel(currentStone_), currentPlayerName());
        } else if (gameOver_) {
            text = "游戏结束";
        } else {
            text = QString("%1｜当前：%2：%3｜比分 黑 %4 : 白 %5")
                       .arg(modeLabel(),
                            stoneLabel(currentStone_),
                            currentPlayerName())
                       .arg(scores_[0])
                       .arg(scores_[1]);
        }
    } else {
        text = gameOver_
            ? "游戏结束"
            : QString("%1｜当前回合：%2：%3").arg(modeLabel(), stoneLabel(currentStone_), currentPlayerName());
    }
    painter.drawText(QRect(0, height() - 52, width(), 28), Qt::AlignCenter, text);

    painter.setFont(QFont("PingFang SC", 11));
    painter.drawText(QRect(0, height() - 26, width(), 22),
                     Qt::AlignCenter,
                     replayMode_ ? (replayPlaybackMode_ == ReplayPlaybackMode::Auto
                                        ? "自动复盘中，可暂停、继续或退出复盘"
                                        : "使用上一步/下一步查看历史落子")
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

void BoardWidget::handleAdvancedClick(int row, int col) {
    if (state_ == InteractionState::Playing) {
        if (!board_.placeStone(row, col, currentStone_)) {
            return;
        }
        replay_.addMove({row, col, currentStone_, currentPlayerName().toStdString()});
        beginLineSelection(row, col);
        return;
    }

    if (state_ == InteractionState::SelectingLine) {
        handleLineEndpointClick(row, col);
        return;
    }

    if (state_ == InteractionState::ConvertingStone) {
        handleConversionClick(row, col);
    }
}

void BoardWidget::handleLineEndpointClick(int row, int col) {
    const gomoku::Position clicked{row, col};
    const bool isEndpoint = std::any_of(selectableLines_.begin(), selectableLines_.end(),
                                        [clicked](const gomoku::FiveLineCandidate& candidate) {
                                            return !candidate.positions.empty() &&
                                                   (candidate.positions.front() == clicked ||
                                                    candidate.positions.back() == clicked);
                                        });
    if (!isEndpoint) {
        QMessageBox::information(this, "选择无效", "请选择彩色线段两端的端点。");
        return;
    }

    if (selectedEndpoints_.empty()) {
        selectedEndpoints_.push_back(clicked);
        update();
        return;
    }

    selectedEndpoints_.push_back(clicked);
    for (const gomoku::FiveLineCandidate& candidate : selectableLines_) {
        if (candidate.positions.empty()) {
            continue;
        }
        const bool sameDirection =
            (candidate.positions.front() == selectedEndpoints_[0] &&
             candidate.positions.back() == selectedEndpoints_[1]) ||
            (candidate.positions.front() == selectedEndpoints_[1] &&
             candidate.positions.back() == selectedEndpoints_[0]);
        if (sameDirection) {
            resolveSelectedLine(candidate);
            return;
        }
    }

    selectedEndpoints_.clear();
    QMessageBox::information(this, "选择无效", "请选择同一条连续五子两端。");
    update();
}

void BoardWidget::handleConversionClick(int row, int col) {
    if (board_.at(row, col) != gomoku::oppositeStone(currentStone_)) {
        QMessageBox::information(this, "选择无效", "请选择一颗对方棋子进行转化。");
        return;
    }

    board_.replaceStone(row, col, currentStone_);
    beginLineSelection(row, col);
}

bool BoardWidget::hasOpponentStone(gomoku::Stone stone) const {
    const gomoku::Stone opponent = gomoku::oppositeStone(stone);
    for (int row = 0; row < kBoardSize; ++row) {
        for (int col = 0; col < kBoardSize; ++col) {
            if (board_.at(row, col) == opponent) {
                return true;
            }
        }
    }
    return false;
}

void BoardWidget::beginLineSelection(int row, int col) {
    selectableLines_ = board_.fiveLineCandidates(row, col);
    selectedEndpoints_.clear();
    if (selectableLines_.empty()) {
        finishAdvancedAction();
        return;
    }

    state_ = InteractionState::SelectingLine;
    saveSnapshot(QString("%1：%2 形成五连，等待选择").arg(stoneLabel(currentStone_), currentPlayerName()));
    updateControlButtons();
    update();
}

void BoardWidget::resolveSelectedLine(const gomoku::FiveLineCandidate& candidate) {
    board_.removeStones(candidate.positions);
    ++scores_[playerIndex(currentStone_)];
    selectableLines_.clear();
    selectedEndpoints_.clear();

    if (scores_[playerIndex(currentStone_)] >= kWinningScore) {
        gameOver_ = true;
        state_ = InteractionState::GameOver;
        saveSnapshot(QString("%1：%2 消除五连并获胜").arg(stoneLabel(currentStone_), currentPlayerName()));
        updateControlButtons();
        update();
        showWinner(currentStone_);
        return;
    }

    if (!hasOpponentStone(currentStone_)) {
        finishAdvancedAction();
        return;
    }

    state_ = InteractionState::ConvertingStone;
    saveSnapshot(QString("%1：%2 消除五连得分，等待转化").arg(stoneLabel(currentStone_), currentPlayerName()));
    updateControlButtons();
    update();
}

void BoardWidget::finishAdvancedAction() {
    state_ = InteractionState::Playing;
    switchPlayer();
    saveSnapshot(QString("切换到 %1：%2").arg(stoneLabel(currentStone_), currentPlayerName()));
}

void BoardWidget::saveSnapshot(const QString& label) {
    history_.push_back({board_, currentStone_, scores_, state_, gameOver_, label});
}

void BoardWidget::restoreSnapshot(const GameSnapshot& snapshot) {
    board_ = snapshot.board;
    currentStone_ = snapshot.currentStone;
    scores_ = snapshot.scores;
    state_ = snapshot.state;
    gameOver_ = snapshot.gameOver;
    selectableLines_.clear();
    selectedEndpoints_.clear();
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

    if (gameMode_ == gomoku::GameMode::AdvancedCapture) {
        if (history_.size() <= 1) {
            QMessageBox::information(this, "无法悔棋", "当前还没有可以撤销的状态。");
            return;
        }
        history_.pop_back();
        restoreSnapshot(history_.back());
        updateControlButtons();
        update();
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
    replayPlaybackMode_ = askReplayPlaybackMode();

    if (gameMode_ == gomoku::GameMode::AdvancedCapture) {
        if (history_.size() <= 1) {
            QMessageBox::information(this, "无法复盘", "当前还没有可以复盘的状态。");
            return;
        }
        replaySnapshots_.clear();
        selectableLines_.clear();
        selectedEndpoints_.clear();
        for (std::size_t i = 1; i < history_.size(); ++i) {
            const GameSnapshot& snapshot = history_[i];
            replaySnapshots_.push_back(snapshot.board);
        }
        replayMode_ = true;
        replayStep_ = 0;
        applyReplayStep();
        if (replayPlaybackMode_ == ReplayPlaybackMode::Auto) {
            startAutoReplay();
        }
        return;
    }

    if (replay_.empty()) {
        QMessageBox::information(this, "无法复盘", "当前还没有可以复盘的落子。");
        return;
    }

    replaySnapshots_ = replay_.snapshots();
    replayMode_ = true;
    replayStep_ = 0;
    applyReplayStep();
    if (replayPlaybackMode_ == ReplayPlaybackMode::Auto) {
        startAutoReplay();
    }
}

void BoardWidget::exitReplayMode() {
    stopAutoReplay();
    replayMode_ = false;
    replayPlaybackMode_ = ReplayPlaybackMode::Manual;
    replayStep_ = 0;
    replayBoard_.reset();
    updateControlButtons();
    update();
}

void BoardWidget::showPreviousReplayStep() {
    if (!replayMode_ || replayStep_ <= 0) {
        return;
    }
    stopAutoReplay();

    --replayStep_;
    applyReplayStep();
}

void BoardWidget::showNextReplayStep() {
    const int maxStep = gameMode_ == gomoku::GameMode::AdvancedCapture
        ? static_cast<int>(replaySnapshots_.size())
        : replay_.size();
    if (!replayMode_ || replayStep_ >= maxStep) {
        return;
    }

    ++replayStep_;
    applyReplayStep();
}

void BoardWidget::toggleAutoReplay() {
    if (!replayMode_ || replayPlaybackMode_ != ReplayPlaybackMode::Auto) {
        return;
    }

    if (autoReplayTimer_->isActive()) {
        stopAutoReplay();
    } else {
        startAutoReplay();
    }
}

void BoardWidget::startAutoReplay() {
    const int maxStep = gameMode_ == gomoku::GameMode::AdvancedCapture
        ? static_cast<int>(replaySnapshots_.size())
        : replay_.size();
    if (!replayMode_ || replayStep_ >= maxStep) {
        updateControlButtons();
        return;
    }

    autoReplayTimer_->start();
    updateControlButtons();
}

void BoardWidget::stopAutoReplay() {
    if (autoReplayTimer_ != nullptr) {
        autoReplayTimer_->stop();
    }
    updateControlButtons();
}

void BoardWidget::advanceAutoReplay() {
    const int maxStep = gameMode_ == gomoku::GameMode::AdvancedCapture
        ? static_cast<int>(replaySnapshots_.size())
        : replay_.size();
    if (!replayMode_ || replayStep_ >= maxStep) {
        stopAutoReplay();
        return;
    }

    ++replayStep_;
    applyReplayStep();
    if (replayStep_ >= maxStep) {
        stopAutoReplay();
    }
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
    const QString winnerText = gameMode_ == gomoku::GameMode::AdvancedCapture
        ? QString("%1：%2 先得 %3 分，获胜！")
              .arg(stoneLabel(winner), playerName(winner))
              .arg(kWinningScore)
        : QString("%1：%2 获胜！").arg(stoneLabel(winner), playerName(winner));
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
