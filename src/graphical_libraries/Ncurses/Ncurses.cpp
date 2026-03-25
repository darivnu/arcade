#include "Ncurses.hpp"
#include <ncurses.h>
#include <string>

static void color_init(void)
{
    init_pair(1, COLOR_RED,     COLOR_BLACK);                                                                                                                                                
    init_pair(2, COLOR_GREEN,   COLOR_BLACK);
    init_pair(3, COLOR_BLUE,    COLOR_BLACK);                                                                                                                                                   
    init_pair(4, COLOR_YELLOW,  COLOR_BLACK);                                                                                                                                                   
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN,    COLOR_BLACK);                                                                                                                                                   
    init_pair(7, COLOR_WHITE,   COLOR_BLACK);
}

void Ncurses::init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);
    start_color();
    color_init();
}

void Ncurses::stop(void)
{
    endwin();
}

const std::string &Ncurses::getName() const
{
    return _name;
}

void Ncurses::drawTile(ShapeType shape, Color color, int x, int y)
{
    char ch = shapeToChar.at(shape);
    int pair = COLOR_PAIR(get_color_pair(color));
                                                                                                                                                                                                
    attron(pair);
    mvaddch(_originY + y, _originX + x, ch);                                                                                                                                                    
    attroff(pair);  
}

int Ncurses::getNcurseWidth()
{
    return getmaxx(stdscr);
}

int Ncurses::getNcurseHeight()
{
    return getmaxy(stdscr);
}

int Ncurses::get_color_pair(Color color)
{
    switch (color) {
      case BLACK:   return 0;                                                                                                                                                                 
      case RED:     return 1;
      case GREEN:   return 2;                                                                                                                                                                 
      case BLUE:    return 3;
      case YELLOW:  return 4;
      case MAGENTA: return 5;                                                                                                                                                                 
      case CYAN:    return 6;
      case WHITE:   return 7;                                                                                                                                                                 
      default:      return 0;
  }
}

void Ncurses::updateLayout()
{
    _originX = (getNcurseWidth() - WIDTH) / 2;
    _originY = (getNcurseHeight() - HEIGHT) / 2;
}

void Ncurses::clear()
{
    erase();
}

EventType Ncurses::pollEvents()
{
    int key = getch();
    switch (key) {
        case 'w': case KEY_UP:    return W_KEY;
        case 'a': case KEY_LEFT:  return A_KEY;                                                                                                                                                 
        case 's': case KEY_DOWN:  return S_KEY;
        case 'd': case KEY_RIGHT: return D_KEY;                                                                                                                                                 
        case ' ':                 return SPACE_KEY;
        case 'm':                 return MENU;                                                                                                                                                  
        case 'q':                 return QUIT;
        default:                  return OTHER;                                                                                                                                                 
    }  
}

void Ncurses::drawText(const std::string& text, Color color, int x, int y)
{
    attron(COLOR_PAIR(get_color_pair(color)));
    mvprintw(y + _originY, x + _originX, "%s", text.c_str());
    attroff(COLOR_PAIR(get_color_pair(color)));
}

void Ncurses::draw()
{
    int termW = getNcurseWidth();
    int termH = getNcurseHeight();

    if (termW < (int)_width + 2 || termH < (int)_height + 2 ) {
        erase();
        std::string msg = "Terminal too small: resize to at least " + std::to_string(_width + 2) + "x" + std::to_string(_height + 2);
        mvprintw(termH / 2, (termW - (int)msg.size()) / 2, "%s", msg.c_str());
        refresh();
        return;
    }

    updateLayout();
    drawBorder();
    refresh();
}


void Ncurses::drawBorder()                                                                                                                                                                  
  {
      int bx = _originX - 1;                                                                                                                                                                  
      int by = _originY - 1;                                                                                                                                                                  
      int pair = COLOR_PAIR(get_color_pair(WHITE));                                                                                                                                           
                                                                                                                                                                                              
      attron(pair);

      mvaddch(by,               bx,               ACS_ULCORNER);                                                                                                                              
      mvaddch(by,               bx + _width + 1,  ACS_URCORNER);
      mvaddch(by + _height + 1, bx,               ACS_LLCORNER);                                                                                                                              
      mvaddch(by + _height + 1, bx + _width + 1,  ACS_LRCORNER);                                                                                                                              
   
      mvhline(by,               bx + 1, ACS_HLINE, _width);                                                                                                                                   
      mvhline(by + _height + 1, bx + 1, ACS_HLINE, _width);
                                                                                                                                                                                              
      mvvline(by + 1, bx,               ACS_VLINE, _height);                                                                                                                                  
      mvvline(by + 1, bx + _width + 1,  ACS_VLINE, _height);
                                                                                                                                                                                              
      attroff(pair);                                                                                                                                                                          
  }

  extern "C" {

IDisplayModule* create()
{
    return new Ncurses();
}

void destroy(IDisplayModule* instance)
{
    instance->stop();
    delete instance;
}

char Ncurses::getInputChar()
{
    int ch = getch();
    if (ch != ERR) {
        if (ch >= 32 && ch <= 126) {
            return static_cast<char>(ch);
        }
        if (ch == '\n' || ch == 27) {
            return '\n';
        }
    }
    return '\0';
}

}
