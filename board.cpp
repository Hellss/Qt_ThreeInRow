#include "board.h"
#include <ctime>
#include <QMessageBox>


Board::Board(QWidget *parent) : QWidget(parent), selectedX(-1), selectedY(-1), canMove(true) {
    srand(time(0));
    gridLayout = new QGridLayout(this);
    init();
    while (tick()) {};
    connect(&timer, &QTimer::timeout, this, &Board::updateBoard);
    timer.start(1250);
}


Board::~Board() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            delete buttons[i][j];
        }
    }
    delete gridLayout;
}


void Board::init() {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            buttons[i][j] = new QPushButton(this);
            grid[i][j] = COLORS[rand() % COLORS.size()];
            buttons[i][j]->setText(grid[i][j]);
            updateButtonAppearance(i, j);  // Set initial color
            buttons[i][j]->setFixedSize(50, 50);
            gridLayout->addWidget(buttons[i][j], i, j);
            connect(buttons[i][j], &QPushButton::clicked, [=]() { handleClick(i, j); });
        }
    }
}


void Board::updateButtonAppearance(int x, int y) {
    QString color = grid[x][y];
    QString backgroundColor = colorMap.value(color, "white");  // white - if not find

    buttons[x][y]->setStyleSheet("background-color: " + backgroundColor + ";");
}


void Board::handleClick(int x, int y) {
    if (!canMove) {
        QMessageBox::information(this, "Can't Move", "Can't move now!");
        return;
    }
    if (selectedX == -1 && selectedY == -1) {
        selectedX = x;
        selectedY = y;
        buttons[x][y]->setStyleSheet("background-color: yellow");
    } else {
        if (isValidMove(selectedX, selectedY, x, y)) {
            swap(selectedX, selectedY, x, y);
            if (!tick()) {
                swap(selectedX, selectedY, x, y);
                QMessageBox::information(this, "Invalid Move", "No combinations - revert operation moving");
            }
        }
        buttons[selectedX][selectedY]->setStyleSheet("");
        updateButtonAppearance(selectedX, selectedY);
        selectedX = -1;
        selectedY = -1;
    }
}


bool Board::isValidMove(int x1, int y1, int x2, int y2) {
    return (abs(x1 - x2) + abs(y1 - y2)) == 1;
}


void Board::swap(int x1, int y1, int x2, int y2) {
    std::swap(grid[x1][y1], grid[x2][y2]);
    buttons[x1][y1]->setText(grid[x1][y1]);
    buttons[x2][y2]->setText(grid[x2][y2]);
    updateButtonAppearance(x1, y1);
    updateButtonAppearance(x2, y2);
}


bool Board::tick() {
    bool changed = false;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (checkAndRemove(i, j)) {
                changed = true;
            }
        }
    }

    canMove = !changed;
    if (changed) {
        collapse();
    }
    return changed;
}


bool Board::checkAndRemove(int x, int y, bool check) {
    QString color = grid[x][y];
    std::vector<std::pair<int, int>> matches;

    // Check horizontal
    int x0 = x;
    int count = 1;
    for (int i = x + 1; i < SIZE && grid[i][y] == color; ++i) {
        count++;
    }
    for (int i = x - 1; i >= 0 && grid[i][y] == color; --i) {
        count++;
        x0 = i;
    }
    if (count >= 3) {
        for (int i = x0; i < x0 + count; ++i) {
            matches.emplace_back(i, y);
        }
    }
    else
    {
        x0 = x;
        count = 1;
    }

    for (int i = x0; i < x0 + count; ++i) {
        // Check vertical
        int y0 = y;
        int count_y = 1;
        for (int j = y + 1; j < SIZE && grid[i][j] == color; ++j) {
            count_y++;
        }
        for (int j = y - 1; j >= 0 && grid[i][j] == color; --j) {
            count_y++;
            y0 = j;
        }
        if (count_y >= 3) {
            for (int j = y0; j < y0 + count_y; ++j) {
                matches.emplace_back(i, j);
            }
        }
    }

    if (!matches.empty() && !check) {
        for (const auto &pos : matches) {
            grid[pos.first][pos.second] = "";
            buttons[pos.first][pos.second]->setText("");
        }
    }
    return !matches.empty();
}


void Board::collapse() {
    for (int j = 0; j < SIZE; ++j) {
        int emptyCells = 0;
        for (int i = SIZE - 1; i >= 0; --i) {
            if (grid[i][j].isEmpty()) {
                emptyCells++;
            } else if (emptyCells > 0) {
                grid[i + emptyCells][j] = grid[i][j];
                grid[i][j] = "";
                buttons[i + emptyCells][j]->setText(grid[i + emptyCells][j]);
                buttons[i][j]->setText("");
                updateButtonAppearance(i + emptyCells, j);
                buttons[i][j]->setStyleSheet("background-color: white;");  // Clear the emptied cells
            }
        }

        for (int i = 0; i < emptyCells; ++i) {
            grid[i][j] = COLORS[rand() % COLORS.size()];
            buttons[i][j]->setText(grid[i][j]);
            updateButtonAppearance(i, j);
        }
    }
}


bool Board::hasPossibleMoves() {
    const std::vector<std::pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    for (int x = 0; x < SIZE; ++x) {
        for (int y = 0; y < SIZE; ++y) {
            for (const auto &dir : directions) {
                int nx = x + dir.first;
                int ny = y + dir.second;
                if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE) {
                    std::swap(grid[x][y], grid[nx][ny]);
                    if (checkAndRemove(x, y, true) || checkAndRemove(nx, ny, true)) {
                        std::swap(grid[x][y], grid[nx][ny]); // revert swap
                        return true;
                    }
                    std::swap(grid[x][y], grid[nx][ny]); // revert swap
                }
            }
        }
    }
    return false;
}


void Board::mix() {
    std::vector<QString> flattened;

    // grid into a 1D vector
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            flattened.push_back(grid[i][j]);
        }
    }

    // Shuffle vector
    std::random_shuffle(flattened.begin(), flattened.end());

    // Rebuild grid from shuffled
    int index = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            grid[i][j] = flattened[index++];
            buttons[i][j]->setText(grid[i][j]);
            updateButtonAppearance(i, j);
        }
    }
}


void Board::updateBoard() {
    while (!hasPossibleMoves()) {
        QMessageBox::information(this, "No Moves Left", "No possible moves left, mixing the board.");
        mix();
    }

    if (tick()) {
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                buttons[i][j]->setText(grid[i][j]);
            }
        }
    }
}
