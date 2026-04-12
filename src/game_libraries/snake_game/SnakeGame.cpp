/*
** EPITECH PROJECT, 2026
** bootstrap_arcade
** File description:
** SnakeGame
*/

#include "SnakeGame.hpp"
#include "IDisplayModule.hpp"
#include "IGameModule.hpp"
#include <cstdlib>

SnakeGame::SnakeGame()
{
    _elapsed = 100;
    int midX = WIDTH / 2;
    int midY = HEIGHT / 2;

    for (int i = 3; i >= 0; i--)
        _snake.push_front({midX - i, midY});
    generateFood();
}

const std::string &SnakeGame::getName() const
{
    return _name;
}

void SnakeGame::tick(EventType input)
{
    _display->clear();

    if (_gameover) {
        _display->drawText("GAME OVER! Score: " + std::to_string(_score) +
                           "  Highscore: " + std::to_string(_highscore), WHITE, 1, 1);
        _display->drawText("Press SPACE to restart", WHITE, 1, 2);
        if (input == SPACE_KEY)
            reset_game(input);
        return;
    }

    // Update direction — no 180° reversal
    if (input == W_KEY && _currentDir != DOWN)
        _nextDir = UP;
    if (input == S_KEY && _currentDir != UP)
        _nextDir = DOWN;
    if (input == A_KEY && _currentDir != RIGHT)
        _nextDir = LEFT;
    if (input == D_KEY && _currentDir != LEFT)
        _nextDir = RIGHT;
    _currentDir = _nextDir;

    // Compute next head position
    int dx = 0, dy = 0;
    if (_currentDir == UP)    dy = -1;
    if (_currentDir == DOWN)  dy =  1;
    if (_currentDir == LEFT)  dx = -1;
    if (_currentDir == RIGHT) dx =  1;

    int nextX = _snake.front().first  + dx;
    int nextY = _snake.front().second + dy;

    // Wall collision
    if (nextX < 0 || nextX >= WIDTH || nextY < 0 || nextY >= HEIGHT) {
        _gameover = true;
        drawSnake();
        return;
    }

    // Body collision (check all segments)
    for (auto &seg : _snake) {
        if (seg.first == nextX && seg.second == nextY) {
            _gameover = true;
            drawSnake();
            return;
        }
    }

    _snake.push_front({nextX, nextY});

    if (nextX == _foodPos.first && nextY == _foodPos.second) {
        _score += 10;
        generateFood();
    } else {
        _snake.pop_back();
    }

    drawSnake();
}

void SnakeGame::exit() {}

void SnakeGame::generateFood()
{
    if (_snake.size() >= (size_t)(WIDTH * HEIGHT)) {
        _gameover = true;
        return;
    }
    std::pair<int,int> pos;
    do {
        pos = {rand() % WIDTH, rand() % HEIGHT};
    } while (checkCollision(pos.first, pos.second));
    _foodPos = pos;
}

void SnakeGame::drawSnake()
{
    // Draw food
    _display->drawSprite({"snake/apple.png", 0, 0, 0, 0, CIRCLE, RED},
                         _foodPos.first, _foodPos.second);

    // Draw head
    std::string headSprite;
    if (_currentDir == UP)    headSprite = "snake/head_up.png";
    if (_currentDir == DOWN)  headSprite = "snake/head_down.png";
    if (_currentDir == LEFT)  headSprite = "snake/head_left.png";
    if (_currentDir == RIGHT) headSprite = "snake/head_right.png";
    _display->drawSprite({headSprite, 0, 0, 0, 0, CIRCLE, GREEN},
                         _snake[0].first, _snake[0].second);

    // Draw body segments
    for (size_t i = 1; i < _snake.size(); i++) {
        auto &seg  = _snake[i];
        auto &prev = _snake[i - 1];

        std::string bodySprite;
        if (prev.first != seg.first)
            bodySprite = "snake/body_horizontal.png";
        else
            bodySprite = "snake/body_vertical.png";

        _display->drawSprite({bodySprite, 0, 0, 0, 0, SQUARE, GREEN},
                             seg.first, seg.second);
    }

    // Draw score
    _display->drawText("Score: " + std::to_string(_score) +
                       "  Highscore: " + std::to_string(_highscore),
                       WHITE, 1, 1);
}

bool SnakeGame::checkCollision(int x, int y)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return true;
    for (auto &seg : _snake) {
        if (seg.first == x && seg.second == y)
            return true;
    }
    return false;
}

void SnakeGame::reset_game(EventType)
{
    if (_score > _highscore)
        _highscore = _score;

    _snake.clear();
    int midX = WIDTH / 2;
    int midY = HEIGHT / 2;
    for (int i = 3; i >= 0; i--)
        _snake.push_front({midX - i, midY});

    _currentDir = RIGHT;
    _nextDir    = RIGHT;
    _gameover   = false;
    _score      = 0;
    generateFood();
}

extern "C" {
    IGameModule* create() { return new SnakeGame(); }
    void destroy(IGameModule* instance) { delete instance; }
}
