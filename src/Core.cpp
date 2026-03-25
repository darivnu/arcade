 #include "Core.hpp"
  #include <chrono>
  #include <thread>

  Core::Core()
  {
      graphical_module = graphical_loader.getInstance();
      game_module = game_loader.getInstance();
  }

  void Core::update_event()
  {
      EventType event = graphical_module->pollEvents();
      if (event != OTHER)
          _lastEvent = event;
  }

  void Core::load_new_game(std::string game_path)
  {
      _menu_game.update_highscore(game_module->getName(), game_module->get_highscore());
      game_module->exit();

      game_loader.setHandle(game_path);
      game_module = game_loader.getInstance();
      game_module->load_display(graphical_module);
      _currentGameLib = game_path;
  }

  void Core::load_new_graphical(std::string graphical_path)
  {
      // CRITICAL: do NOT call graphical_module->stop() here manually.
      // setHandle() → reset() → _destroyFunc() already calls stop() + delete.
      // Calling stop() first = double-stop = crash.
      graphical_loader.setHandle(graphical_path);
      graphical_module = graphical_loader.getInstance();
      graphical_module->init();
      game_module->load_display(graphical_module);
      _menu_game.load_display(graphical_module);
      _currentGraphicalLib = graphical_path;
  }

  void Core::menu_handle()
  {
      if (_lastEvent == QUIT) {
          _running = false;
          return;
      }

      _menu_game.tick(_lastEvent);

      auto [game_path, graphical_path] = _menu_game.get_path_chosen();
      if (!game_path.empty() && !graphical_path.empty()) {
          bool launch_game = true;

          if (game_path != _currentGameLib)
              load_new_game(game_path);

          if (graphical_path != _currentGraphicalLib) {
              load_new_graphical(graphical_path);
              launch_game = false;
          }

          if (launch_game) {
              _elapsed = game_module->get_elapsed();
              _menu = false;
          } else {
              _elapsed = _menu_game.get_elapsed();
          }
      }

      if (player_name.empty())
          player_name = _menu_game.get_player_name();
  }

  void Core::run()
  {
      game_module->load_display(graphical_module);
      _menu_game.load_display(graphical_module);
      _elapsed = _menu_game.get_elapsed();
      graphical_module->init();

      _lastMoveTime = std::chrono::steady_clock::now();

      while (_running) {
          update_event();

          // QUIT or MENU while in game → go to menu first, quit on second press
          if ((_lastEvent == QUIT || _lastEvent == MENU) && !_menu) {
              _menu = true;
              _menu_game.update_highscore(game_module->getName(), game_module->get_highscore());
              _elapsed = _menu_game.get_elapsed();
              _lastEvent = OTHER;
          }

          auto now = std::chrono::steady_clock::now();
          long ms_passed = std::chrono::duration_cast<std::chrono::milliseconds>(
              now - _lastMoveTime).count();

          if ((unsigned long)ms_passed < _elapsed) {
              std::this_thread::sleep_for(std::chrono::milliseconds(_elapsed - ms_passed));
              continue;
          }

          graphical_module->clear();

          if (_menu)
              menu_handle();
          else
              game_module->tick(_lastEvent);

          _lastEvent = OTHER;
          _lastMoveTime = std::chrono::steady_clock::now();

          graphical_module->draw();
      }
      // Do not call graphical_module->stop() here:
      // DLLoader's destructor/reset() will invoke the library's destroy()
      // hook, and current implementations call stop() there. Calling it
      // again here would double-stop and may crash (e.g., ncurses).
  }