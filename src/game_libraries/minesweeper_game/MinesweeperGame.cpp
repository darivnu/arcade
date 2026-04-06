#include "MinesweeperGame.hpp"
#include "IDisplayModule.hpp"
#include "IGameModule.hpp"
#include <algorithm>
#include <vector>


MinesweeperGame::MinesweeperGame() : _cursorX(MINE_W / 2), _cursorY(MINE_H / 2), _firstClick(true), _won(false), _loseX(0), _loseY(0)
{
    _elapsed = 50;
    initBoard();
}

const std::string &MinesweeperGame::getName() const
{
    return _name;
}

void MinesweeperGame::tick(EventType input) {
    if (_gameover) {
        reset_game();
        return;
    }
    if (input == W_KEY && _cursorY > 0)
        _cursorY--;
    if (input == S_KEY && _cursorY < MINE_H - 1)
        _cursorY++;
    if (input == A_KEY && _cursorX > 0)
        _cursorX--;
    if (input == D_KEY && _cursorX < MINE_W - 1)
        _cursorX++;
    if (input == SPACE_KEY && !_flagged[_cursorX][_cursorY]) {
        if (_firstClick) {
            placeMines(_cursorX, _cursorY);
            computeAdjacency();
            _firstClick = false;
        }
        if (_mines[_cursorX][_cursorY]) {
            _loseX = _cursorX;
            _loseY = _cursorY;
            _gameover = true;
            _won = false;
        }
        if (!_mines[_cursorX][_cursorY]) {
            reveal(_cursorX, _cursorY);
            if (checkWin()) {
                _gameover = true;
                _won = true;
            }
        }
    }
    if (input == MOUSE_R && !_revealed[_cursorX][_cursorY])
        _flagged[_cursorX][_cursorY] = !_flagged[_cursorX][_cursorY];
    drawBoard();
}

void MinesweeperGame::exit() {}

void MinesweeperGame::initBoard()
{
    _mines.assign(MINE_W, std::vector<bool>(MINE_H, false));
    _revealed.assign(MINE_W, std::vector<bool>(MINE_H, false));
    _flagged.assign(MINE_W, std::vector<bool>(MINE_H, false));
    _adjacentCounts.assign(MINE_W, std::vector<int>(MINE_H, 0));
}

void MinesweeperGame::drawBoard()
{
    int offsetX = (WIDTH - MINE_W) / 2;                    // center grid horizontally
    int offsetY = (HEIGHT - MINE_H) / 2;                   // center grid vertically

    for (int x = 0; x < MINE_W; x++) {
        for (int y = 0; y < MINE_H; y++) {
            int px = offsetX + x;                          // final screen x
            int py = offsetY + y;                          // final screen y

            if (_gameover && _mines[x][y] && x == _loseX && y == _loseY)       // the mine the player hit
                _display->drawSprite({"minesweeper/board.png", 96, 112, 16, 16, CIRCLE, RED}, px, py);
            if (_gameover && _mines[x][y] && !(x == _loseX && y == _loseY))    // all other mines revealed on game over
                _display->drawSprite({"minesweeper/board.png", 80, 112, 16, 16, CIRCLE, WHITE}, px, py);
            if (!(_gameover && _mines[x][y]) && _flagged[x][y])                // flagged cell (not a revealed mine)
                _display->drawSprite({"minesweeper/board.png", 32, 112, 16, 16, SQUARE, YELLOW}, px, py);
            if (!(_gameover && _mines[x][y]) && !_flagged[x][y] && !_revealed[x][y])  // hidden unrevealed cell
                _display->drawSprite({"minesweeper/board.png", 16, 112, 16, 16, SQUARE, CYAN}, px, py);
            if (!(_gameover && _mines[x][y]) && !_flagged[x][y] && _revealed[x][y] && _adjacentCounts[x][y] == 0)  // revealed empty
                _display->drawSprite({"minesweeper/board.png", 0, 96, 16, 16, SQUARE, BLACK}, px, py);
            if (!(_gameover && _mines[x][y]) && !_flagged[x][y] && _revealed[x][y] && _adjacentCounts[x][y] == 8) {  // number 8
                _display->drawSprite({"minesweeper/board.png", 0, 112, 16, 16, SQUARE, WHITE}, px, py);
                _display->drawText(std::to_string(_adjacentCounts[x][y]), WHITE, px, py);
            }
            if (!(_gameover && _mines[x][y]) && !_flagged[x][y] && _revealed[x][y] && _adjacentCounts[x][y] > 0 && _adjacentCounts[x][y] < 8) {  // numbers 1-7
                _display->drawSprite({"minesweeper/board.png", (uint8_t)((_adjacentCounts[x][y]) * 16), 96, 16, 16, SQUARE, WHITE}, px, py);
                _display->drawText(std::to_string(_adjacentCounts[x][y]), WHITE, px, py);
            }
        }
    }
    if (!_gameover) {
        _display->drawSprite({"minesweeper/board.png", 0, 192, 16, 16, SQUARE, MAGENTA}, offsetX + _cursorX, offsetY + _cursorY);
    }
}

void MinesweeperGame::placeMines(int safeX, int safeY)
{
    int placed = 0;

    while (placed < MINE_COUNT) {
        int x = rand() % MINE_W;
        int y = rand() % MINE_H;
        if (_mines[x][y])
            continue;
        if (abs(x-safeX) <= 1 && abs(y - safeY) <= 1)
            continue;
        _mines[x][y] = true;
        placed++;
    }
}

void MinesweeperGame::computeAdjacency()
{
    int i = 0;
    while (i < MINE_W * MINE_H) {
        int x = i % MINE_W;
        int y = i / MINE_W;
        if (_mines[x][y]) {
            if (x > 0 && y > 0)                _adjacentCounts[x-1][y-1]++;
            if (y > 0)                         _adjacentCounts[x][y-1]++;
            if (x < MINE_W-1 && y > 0)        _adjacentCounts[x+1][y-1]++;
            if (x > 0)                         _adjacentCounts[x-1][y]++;
            if (x < MINE_W-1)                  _adjacentCounts[x+1][y]++;
            if (x > 0 && y < MINE_H-1)        _adjacentCounts[x-1][y+1]++;
            if (y < MINE_H-1)                  _adjacentCounts[x][y+1]++;
            if (x < MINE_W-1 && y < MINE_H-1) _adjacentCounts[x+1][y+1]++;
        }
        i++;
    }
}

void MinesweeperGame::reveal(int x, int y)
{
    if (x < 0 || x >= MINE_W || y < 0 || y >= MINE_H)
        return;
    if (_revealed[x][y] || _flagged[x][y])
        return;
    _revealed[x][y] = true;
    _score += 10;
    if (_adjacentCounts[x][y] != 0)
        return;
    reveal(x - 1, y - 1);
    reveal(x,     y - 1);
    reveal(x + 1, y - 1);
    reveal(x - 1, y);
    reveal(x + 1, y);
    reveal(x - 1, y + 1);
    reveal(x,     y + 1);
    reveal(x + 1, y + 1);
}

bool MinesweeperGame::checkWin()
{
    int i = 0;
    while (i < MINE_W * MINE_H) {
        int x = i % MINE_W;
        int y = i / MINE_W;
        if (!_mines[x][y] && !_revealed[x][y])
            return false;
        i++;
    }
    return true;
}

void MinesweeperGame::reset_game()
{
    _display->clear();
    drawBoard();
    if (_won)
        _display->drawText("YOU WIN! Score: " + std::to_string(_score), GREEN, 1, 1);
    if (!_won)
        _display->drawText("GAME OVER! Score: " + std::to_string(_score), RED, 1, 1);
    _display->drawText("Press SPACE to restart", WHITE, 1, 2);
    _display->draw();

    if (_score > _highscore)
        _highscore = _score;

    EventType ev = OTHER;
    while (ev != SPACE_KEY && ev != QUIT && ev != MENU)
        ev = _display->pollEvents();

    initBoard();
    _firstClick = true;
    _won = false;
    _gameover = false;
    _score = 0;
    _cursorX = MINE_W / 2;
    _cursorY = MINE_H / 2;
}


extern "C" {
      IGameModule* create() { return new MinesweeperGame(); }
      void destroy(IGameModule* instance) { delete instance; }
  }
