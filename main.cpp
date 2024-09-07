#include "board.h"
#include <QApplication>
#include <QIcon>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Board board;
    board.setWindowTitle("Three In Row");
    board.setWindowIcon(QIcon(":/resources/icon.png"));
    board.show();
    return app.exec();
}
