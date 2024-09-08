#include "board.h"
#include <set>
#include <ctime>
#include <QMessageBox>


Board::Board(QWidget *parent) : QWidget(parent), selectedX(-1), selectedY(-1), canMove(true), score(-1) {
    srand(time(0));
    gridLayout = new QGridLayout(this);
    init();
    while (tick()) {};
    score = 0;
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
            //buttons[i][j]->setFixedSize(45, 45);
            buttons[i][j]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            grid[i][j] = COLORS[rand() % COLORS.size()];
            buttons[i][j]->setText(grid[i][j]);
            updateButtonAppearance(i, j);  // Set initial color
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
    if (color == "")
        return false;

    std::set<std::pair<int, int>> matches;  // Set для хранения уникальных совпадений

    // Check horizontal
    int count = 1;
    int xStart = x;
    int xEnd = x;
    for (int i = x + 1; i < SIZE && grid[i][y] == color; ++i) {
        count++;
        xEnd = i;
    }
    for (int i = x - 1; i >= 0 && grid[i][y] == color; --i) {
        count++;
        xStart = i;
    }
    if (count >= 3) {
        for (int i = xStart; i <= xEnd; ++i) {
            matches.emplace(i, y);  // Добавляем все элементы линии в сет
        }
    }

    // Check vertical
    count = 1;
    int yStart = y;
    int yEnd = y;
    for (int j = y + 1; j < SIZE && grid[x][j] == color; ++j) {
        count++;
        yEnd = j;
    }
    for (int j = y - 1; j >= 0 && grid[x][j] == color; --j) {
        count++;
        yStart = j;
    }
    if (count >= 3) {
        for (int j = yStart; j <= yEnd; ++j) {
            matches.emplace(x, j);  // Добавляем все элементы вертикальной линии в сет
        }
    }

    // Проверка на "крест" или "угол"
    for (auto& match : matches) {
        int mx = match.first;
        int my = match.second;

        // Проверяем горизонтальные линии вокруг вертикальных совпадений
        if (mx > 1 && grid[mx - 1][my] == color && grid[mx - 2][my] == color) {
            matches.emplace(mx - 1, my);
            matches.emplace(mx - 2, my);
        }
        if (mx < SIZE - 2 && grid[mx + 1][my] == color && grid[mx + 2][my] == color) {
            matches.emplace(mx + 1, my);
            matches.emplace(mx + 2, my);
        }

        // Проверяем вертикальные линии вокруг горизонтальных совпадений
        if (my > 1 && grid[mx][my - 1] == color && grid[mx][my - 2] == color) {
            matches.emplace(mx, my - 1);
            matches.emplace(mx, my - 2);
        }
        if (my < SIZE - 2 && grid[mx][my + 1] == color && grid[mx][my + 2] == color) {
            matches.emplace(mx, my + 1);
            matches.emplace(mx, my + 2);
        }
    }

    if (!matches.empty() && !check) {
        if (score != -1) {
            score += matches.size();
            setWindowTitle("Three In Row. Score: " + QString::number(score));
        }
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
