#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include "Gomoku/AI.h"
#include "Gomoku/Board.h"
#include "Gomoku/Player.h"
#include "Gomoku/Replay.h"

#include <QPoint>
#include <QString>
#include <QWidget>

#include <array>
#include <memory>
#include <vector>

class QPushButton;
class QTimer;

// BoardWidget 是 GUI 层的集中控制器：负责绘制、鼠标输入、按钮状态、
// 普通/进阶模式流程、悔棋和复盘。核心棋盘规则仍委托给 gomoku::Board。
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

    // 进阶模式状态机：Playing→(成五)SelectingLine→(消除)ConvertingStone→(成五链)SelectingLine/Playing→(3分)GameOver
    enum class InteractionState {
        Playing,
        SelectingLine,
        ConvertingStone,
        GameOver
    };

    enum class ReplayPlaybackMode {
        Manual,
        Auto
    };

    // 进阶模式一次操作可能包含落子、消除、得分、转化和连锁。
    // 因此悔棋/复盘不能只记录 Move，而要保存完整游戏状态快照。
    struct GameSnapshot {
        gomoku::Board board;
        gomoku::Stone currentStone;
        std::array<int, 2> scores;
        InteractionState state;
        bool gameOver;
        QString label;
    };

    std::unique_ptr<gomoku::AI> ai_;
    gomoku::Board board_;
    gomoku::Replay replay_;

    // 复盘显示专用棋盘：进入复盘后 drawStones() 读取 replayBoard_，
    // 避免复盘前进/后退时直接改动真实对局棋盘 board_。
    gomoku::Board replayBoard_;
    std::vector<gomoku::Board> replaySnapshots_;
    gomoku::Stone currentStone_ = gomoku::Stone::Black;
    gomoku::GameMode gameMode_ = gomoku::GameMode::Classic;
    std::array<gomoku::Player, 2> players_ = {
        gomoku::Player("黑方玩家", gomoku::Stone::Black),
        gomoku::Player("白方玩家", gomoku::Stone::White)};
    std::array<int, 2> scores_ = {0, 0};
    InteractionState state_ = InteractionState::Playing;
    bool gameOver_ = false;
    bool replayMode_ = false;
    ReplayPlaybackMode replayPlaybackMode_ = ReplayPlaybackMode::Manual;
    int replayStep_ = 0;
    QTimer* autoReplayTimer_ = nullptr;

    // 仅进阶模式使用的完整快照栈。普通模式使用 replay_ 的落子列表即可。
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
    int maxReplayStep() const;
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
    ReplayPlaybackMode askReplayPlaybackMode();
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
    void toggleAutoReplay();
    void startAutoReplay();
    void stopAutoReplay();
    void advanceAutoReplay();
    void applyReplayStep();
    void showWinner(gomoku::Stone winner);
    void triggerAITurn();
    void executeAIMove();
};

#endif // BOARDWIDGET_H
