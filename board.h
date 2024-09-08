#ifndef BOARD_H
#define BOARD_H


#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include <vector>


namespace {
    const int SIZE = 10;
    const std::vector<QString> COLORS = {"A", "B", "C", "D", "E", "F"};
    const QMap<QString, QString> colorMap = {
        {"A", "red"},
        {"B", "blue"},
        {"C", "green"},
        {"D", "pink"},
        {"E", "purple"},
        {"F", "orange"}
    };
}


class Board : public QWidget
{
    Q_OBJECT

public:
    Board(QWidget *parent = nullptr);
    ~Board();
private:
    QGridLayout *gridLayout;
    QPushButton *buttons[SIZE][SIZE];
    QString grid[SIZE][SIZE];
    QTimer timer;
    int selectedX, selectedY;
    bool canMove;
    size_t score;

    void init();
    void updateButtonAppearance(int x, int y);    
    void handleClick(int x, int y);
    bool isValidMove(int x1, int y1, int x2, int y2);
    void swap(int x1, int y1, int x2, int y2);
    bool tick();
    bool checkAndRemove(int x, int y, bool check = false);
    void collapse();
    void mix();
    bool hasPossibleMoves();
    void updateBoard();
};
#endif // BOARD_H
