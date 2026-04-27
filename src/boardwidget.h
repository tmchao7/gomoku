#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include "Gomoku/Board.h"

#include <QPoint>
#include <QWidget>

class BoardWidget : public QWidget {
public:
    explicit BoardWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    static constexpr int kBoardSize = gomoku::kBoardSize;
    static constexpr int kMargin = 42;
    static constexpr int kStonePadding = 4;

    gomoku::Board board_;
    gomoku::Stone currentStone_ = gomoku::Stone::Black;
    bool gameOver_ = false;

    int cellSize() const;
    int boardPixelSize() const;
    QPoint gridToPixel(int row, int col) const;
    bool pixelToGrid(const QPoint& position, int& row, int& col) const;

    void drawBoard(QPainter& painter);
    void drawStones(QPainter& painter);
    void drawStatusText(QPainter& painter);
    void placeStone(int row, int col);
    void switchPlayer();
    void showWinner(gomoku::Stone winner);
};

#endif // BOARDWIDGET_H
