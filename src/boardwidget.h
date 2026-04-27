#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include "Gomoku/Board.h"
#include "Gomoku/Replay.h"

#include <QPoint>
#include <QString>
#include <QWidget>

#include <array>
#include <vector>

class QPushButton;

class BoardWidget : public QWidget {
public:
    explicit BoardWidget(QWidget* parent = nullptr);
    void startNewGame();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    static constexpr int kBoardSize = gomoku::kBoardSize;
    static constexpr int kMargin = 42;
    static constexpr int kStonePadding = 4;
    static constexpr int kWinningScore = 3;

    enum class InteractionState {
        Playing,
        SelectingLine,
        ConvertingStone,
        GameOver
    };

    struct GameSnapshot {
        gomoku::Board board;
        gomoku::Stone currentStone;
        std::array<int, 2> scores;
        InteractionState state;
        bool gameOver;
        QString label;
    };

    gomoku::Board board_;
    gomoku::Replay replay_;
    gomoku::Board replayBoard_;
    std::vector<gomoku::Board> replaySnapshots_;
    gomoku::Stone currentStone_ = gomoku::Stone::Black;
    gomoku::GameMode gameMode_ = gomoku::GameMode::Classic;
    std::array<QString, 2> playerNames_ = {"黑方玩家", "白方玩家"};
    std::array<int, 2> scores_ = {0, 0};
    InteractionState state_ = InteractionState::Playing;
    bool gameOver_ = false;
    bool replayMode_ = false;
    int replayStep_ = 0;
    std::vector<GameSnapshot> history_;
    std::vector<gomoku::FiveLineCandidate> selectableLines_;
    std::vector<gomoku::Position> selectedEndpoints_;
    QPushButton* undoButton_ = nullptr;
    QPushButton* resignButton_ = nullptr;
    QPushButton* replayButton_ = nullptr;
    QPushButton* restartButton_ = nullptr;

    int cellSize() const;
    int boardPixelSize() const;
    QPoint gridToPixel(int row, int col) const;
    bool pixelToGrid(const QPoint& position, int& row, int& col) const;
    int playerIndex(gomoku::Stone stone) const;
    QString currentPlayerName() const;
    QString playerName(gomoku::Stone stone) const;
    QString stoneLabel(gomoku::Stone stone) const;
    QString modeLabel() const;
    gomoku::Stone displayedStoneAt(int row, int col) const;

    void setupControls();
    void updateControlButtons();
    void askPlayerNames();
    void askGameMode();
    void drawBoard(QPainter& painter);
    void drawSelectableLines(QPainter& painter);
    void drawStones(QPainter& painter);
    void drawStatusText(QPainter& painter);
    void placeStone(int row, int col);
    void handleAdvancedClick(int row, int col);
    void handleLineEndpointClick(int row, int col);
    void handleConversionClick(int row, int col);
    bool hasOpponentStone(gomoku::Stone stone) const;
    void beginLineSelection(int row, int col);
    void resolveSelectedLine(const gomoku::FiveLineCandidate& candidate);
    void finishAdvancedAction();
    void saveSnapshot(const QString& label);
    void restoreSnapshot(const GameSnapshot& snapshot);
    void switchPlayer();
    void undoLastMove();
    void resignCurrentPlayer();
    void enterReplayMode();
    void exitReplayMode();
    void showPreviousReplayStep();
    void showNextReplayStep();
    void applyReplayStep();
    void showWinner(gomoku::Stone winner);
};

#endif // BOARDWIDGET_H
