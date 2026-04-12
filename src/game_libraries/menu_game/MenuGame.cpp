#include "MenuGame.hpp"
#include "IDisplayModule.hpp"
#include <dirent.h>
#include "Error.hpp"

// ─── Constructor ──────────────────────────────────────────────────────────────

MenuGame::MenuGame(std::string default_game_path, std::string default_graphical_path)
{
    loadLibs();

    currentcore_game_path      = default_game_path;
    currentcore_graphical_path = default_graphical_path;

    // _gameLibs/_graphicalLibs store plain filenames (e.g. "arcade_snake.so").
    // Match the default paths by comparing against the basename of each default.
    std::string default_game_name    = default_game_path.substr(default_game_path.find_last_of("/\\") + 1);
    std::string default_graphical_name = default_graphical_path.substr(default_graphical_path.find_last_of("/\\") + 1);

    for (size_t i = 0; i < _gameLibs.size(); i++) {
        if (_gameLibs[i] == default_game_name) {
            selected_game_index = i;
            break;
        }
    }
    for (size_t i = 0; i < _graphicalLibs.size(); i++) {
        if (_graphicalLibs[i] == default_graphical_name) {
            selected_graphical_index = i;
            break;
        }
    }
}

// ─── IGameModule overrides ────────────────────────────────────────────────────

const std::string &MenuGame::getName() const
{
    return _name;
}

// load_display is the entry point from Core — it initialises the display and
// immediately runs the name-input phase before returning control to Core's loop.
void MenuGame::load_display(IDisplayModule* display)
{
    _display = display;
    _display->init();

    _width  = _display->getWidth();
    _height = _display->getHeight();

    handle_name_input(); // blocking: collects player name before the menu renders
}

void MenuGame::exit()
{
    write_highscore_file();
}

// ─── Library discovery ────────────────────────────────────────────────────────

// Stores plain filenames only — full paths are built at selection time.
void MenuGame::loadLibs()
{
    DIR *dir;
    struct dirent *ent;

    dir = opendir("./lib/game_lib");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string name = ent->d_name;
            if (name.size() > 3 && name.substr(name.size() - 3) == ".so")
                _gameLibs.push_back(name);
        }
        closedir(dir);
    } else {
        throw CoreError("Failed to open directory: ./lib/game_lib");
    }

    dir = opendir("./lib/graphical_lib");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string name = ent->d_name;
            if (name.size() > 3 && name.substr(name.size() - 3) == ".so")
                _graphicalLibs.push_back(name);
        }
        closedir(dir);
    } else {
        throw CoreError("Failed to open directory: ./lib/graphical_lib");
    }
}

// ─── Name-input phase (blocking, runs before the main menu loop) ──────────────
//
// Uses ONLY getInputChar() — never pollEvents() — because both call getch()
// internally and consuming the same keystroke twice would silently drop input.
// getInputChar() returns '\n' for the Enter key.
void MenuGame::handle_name_input()
{
    while (!name_entered) {
        char ch = _display->getInputChar();

        if (ch == '\n') {          // Enter confirms the name (empty name is valid)
            name_entered = true;
            break;
        }
        if (ch == 127 || ch == 8) { // backspace / DEL
            if (!player_name.empty())
                player_name.pop_back();
        } else if (ch != '\0') {   // printable character
            player_name += ch;
        }

        _display->clear();
        _display->drawText("Enter your name:", WHITE, 0, 0);
        _display->drawText(player_name + "_", YELLOW, 0, 1);
        _display->draw();
    }

    read_highscore_file();
}

// ─── Highscore file I/O ───────────────────────────────────────────────────────

void MenuGame::read_highscore_file()
{
    std::ifstream file(highscore_file);
    if (!file.is_open()) {
        highscore_json = nlohmann::json::object();
        return;
    }
    try {
        file >> highscore_json;
    } catch (...) {
        highscore_json = nlohmann::json::object();
    }
}

void MenuGame::write_highscore_file()
{
    std::ofstream file(highscore_file);
    if (!file.is_open()) return;
    file << highscore_json.dump(4);
}

void MenuGame::update_highscore(std::string game_name, unsigned int highscore)
{
    if (highscore_json.contains(game_name) &&
        highscore_json[game_name].contains(player_name)) {
        if (highscore > highscore_json[game_name][player_name])
            highscore_json[game_name][player_name] = highscore;
    } else {
        highscore_json[game_name][player_name] = highscore;
    }
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────

// Draws each row as a single string to keep character widths consistent.
void MenuGame::drawBox(int startX, int startY, int width, int height)
{
    if (width  > WIDTH)  width  = WIDTH;
    if (height > HEIGHT) height = HEIGHT;

    std::string hline = "+" + std::string(width - 2, '-') + "+";
    std::string side  = "|" + std::string(width - 2, ' ') + "|";

    _display->drawText(hline, WHITE, startX, startY);
    for (int i = 1; i < height - 1; i++)
        _display->drawText(side, WHITE, startX, startY + i);
    _display->drawText(hline, WHITE, startX, startY + height - 1);
}

// Shows the current player's scores across all games.
void MenuGame::displayHighscores(int startX, int startY)
{
    _display->drawText("Highscores:", CYAN, startX, startY + 1);

    if (highscore_json.empty() || player_name.empty()) {
        _display->drawText("No scores yet.", WHITE, startX + 2, startY + 3);
        return;
    }

    int row  = startY + 3;
    int rank = 1;
    for (auto& [game_name, scores] : highscore_json.items()) {
        if (!scores.contains(player_name)) continue;
        unsigned int score = scores[player_name];
        _display->drawText(
            std::to_string(rank) + ". " + game_name + ": " + std::to_string(score),
            YELLOW, startX + 2, row++);
        if (++rank > 6) break;
    }
}

// ─── Library cycling (NUM_1/2/3/4 mid-game switching via Core) ───────────────

std::string MenuGame::get_next_game(bool previous)
{
    int n = (int)_gameLibs.size();
    if (n == 0) return currentcore_game_path;
    selected_game_index = previous
        ? (selected_game_index - 1 + n) % n
        : (selected_game_index + 1) % n;
    return "./lib/game_lib/" + _gameLibs[selected_game_index];
}

std::string MenuGame::get_next_graphical(bool previous)
{
    int n = (int)_graphicalLibs.size();
    if (n == 0) return currentcore_graphical_path;
    selected_graphical_index = previous
        ? (selected_graphical_index - 1 + n) % n
        : (selected_graphical_index + 1) % n;
    return "./lib/graphical_lib/" + _graphicalLibs[selected_graphical_index];
}

// ─── Path selection result (polled by Core each menu tick) ───────────────────

std::tuple<std::string, std::string> MenuGame::get_path_chosen()
{
    if (!selected_option)
        return {"", ""};
    selected_option = false;
    std::string game_path    = "./lib/game_lib/"      + _gameLibs[selected_game_index];
    std::string graphical_path = "./lib/graphical_lib/" + _graphicalLibs[selected_graphical_index];
    return {game_path, graphical_path};
}

// ─── Main tick (called by Core every _elapsed ms while in the menu) ──────────

void MenuGame::tick(EventType input)
{
    if (!_display) return;

    // Single global_index spans all gameLibs then all graphicalLibs.
    int maxIndex = (int)(_gameLibs.size() + _graphicalLibs.size()) - 1;

    if (input == W_KEY && global_index > 0)
        global_index--;
    if (input == S_KEY && global_index < maxIndex)
        global_index++;

    // SPACE selects whichever item is highlighted.
    // ENTER is not used here because pollEvents() maps '\n' to OTHER in ncurses.
    if (input == SPACE_KEY) {
        if (global_index < (int)_gameLibs.size())
            selected_game_index = global_index;
        else
            selected_graphical_index = global_index - (int)_gameLibs.size();
        selected_option = true;
    }

    // Keep the per-panel cursor indices in sync with global_index.
    if (global_index < (int)_gameLibs.size())
        current_game_index = global_index;
    else
        current_graphical_index = global_index - (int)_gameLibs.size();

    // ── Rendering ──────────────────────────────────────────────────────────
    _display->clear();

    int y     = 0;
    int index = 0;

    _display->drawText("=== ARCADE MENU ===",      CYAN,    0, y++);
    _display->drawText("Player: " + player_name,   YELLOW,  0, y++);
    _display->drawText("W/S: navigate   SPACE: select   Q: quit", GREEN, 0, y++);
    y++;

    _display->drawText("-- Game Libraries --", MAGENTA, 0, y++);
    for (const auto& lib : _gameLibs) {
        std::string prefix = (global_index == index) ? "> " : "  ";
        Color color = (global_index == index) ? GREEN : WHITE;
        _display->drawText(prefix + lib, color, 2, y++);
        index++;
    }
    y++;

    _display->drawText("-- Graphical Libraries --", MAGENTA, 0, y++);
    for (const auto& lib : _graphicalLibs) {
        std::string prefix = (global_index == index) ? "> " : "  ";
        Color color = (global_index == index) ? GREEN : WHITE;
        _display->drawText(prefix + lib, color, 2, y++);
        index++;
    }

    displayHighscores(0, y + 1);
}
