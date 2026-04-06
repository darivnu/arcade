#include "AGameModule.hpp"
#include <vector>
#include <string>

inline constexpr int MINE_W = 20;
inline constexpr int MINE_H = 18;
inline constexpr int MINE_COUNT = 40;

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

