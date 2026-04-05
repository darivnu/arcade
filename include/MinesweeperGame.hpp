#include "AGameModule.hpp"
#include <vector>
#include <string>

#define MINE_W 20
#define MINE_H 18
#define MINE_COUNT 40

class MinesweeperGame : public AGameModule {
    private:
        const std::string _name = "MinesweeperGame";
        std::vector<std::vector<bool>> _mines;
        std::vector<std::vector<bool>> _revealed;
        std::vector<std::vector<bool>> _flagged;
        std::vector<std::vector<int>> _adjacentCounts;

        int _cursorX;
        int _cursorY;
        bool _firstClick;
        bool _won;
        int _loseX;
        int _loseY;

        void initBoard();
        void placeMines(int safeX, int safeY);
        void computeAdjacency();
        void reveal(int x, int y);
        void drawBoard();
        bool checkWin();
        void reset_game();

    public:
        MinesweeperGame();
        const std::string &getName() const override;
        void tick(EventType input) override;
        void exit() override;
};

