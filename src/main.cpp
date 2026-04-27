#include "boardwidget.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    BoardWidget boardWidget;
    boardWidget.setWindowTitle("五子棋");
    boardWidget.show();

    return app.exec();
}
