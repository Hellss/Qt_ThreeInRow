#include "board.h"
#include <QApplication>
#include <QIcon>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Board board;
    board.setWindowTitle("Three In Row. Score: 0");
    board.setWindowIcon(QIcon(":/resources/icon.png"));
    board.resize(500, 500);
    board.show();
    return app.exec();
}
