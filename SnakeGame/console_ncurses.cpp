#include "console.h"
#include <ncurses.h>
#include <cstdarg>
#include <cstdio>
#include <unistd.h>

enum {
    COLOR_COUNT      = 16,
    COLOR_MASK       = 0x0f,
    COLOR_BRIGHT_MIN = 8,
    COLOR_PAIR_BASE  = 1,
    WRITE_BUF_SIZE   = 512,
    US_PER_MS        = 1000
};

static void initColorPairs()
{
    static const short kNcursesColor[COLOR_COUNT] = {
        COLOR_BLACK, COLOR_BLUE,    COLOR_GREEN,   COLOR_CYAN,
        COLOR_RED,   COLOR_MAGENTA, COLOR_YELLOW,  COLOR_WHITE,
        COLOR_BLACK, COLOR_BLUE,    COLOR_GREEN,   COLOR_CYAN,
        COLOR_RED,   COLOR_MAGENTA, COLOR_YELLOW,  COLOR_WHITE
    };
    for (int i = 0; i < COLOR_COUNT; i++)
        init_pair(i + COLOR_PAIR_BASE, kNcursesColor[i], COLOR_BLACK);
}

void ConInit(int /*width*/, int /*height*/)
{
    initscr();
    set_escdelay(25);
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);
    curs_set(0);
    initColorPairs();
}

void ConShutdown()
{
    endwin();
}

void ConClear()
{
    clear();
    refresh();
}

void ConSleep(int ms)
{
    usleep(ms * US_PER_MS);
}

void ConFlush()
{
    refresh();
}

void ConGotoXY(int x, int y)
{
    move(y, x);
}

void ConTextColor(int color)
{
    int pair = (color & COLOR_MASK) + COLOR_PAIR_BASE;
    if (color >= COLOR_BRIGHT_MIN)
        attrset(A_BOLD | COLOR_PAIR(pair));
    else
        attrset(COLOR_PAIR(pair));
}

void ConResizeConsole(int /*width*/, int /*height*/)
{
}

int ConWriteXY(int x, int y, int color, const char* fmt, ...)
{
    ConGotoXY(x, y);
    ConTextColor(color);

    char buf[WRITE_BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    printw("%s", buf);
    return n;
}

int ConGetch()
{
    int c = getch();
    switch (c) {
        case KEY_UP:    return CON_KEY_ARROW_UP;
        case KEY_DOWN:  return CON_KEY_ARROW_DOWN;
        case KEY_LEFT:  return CON_KEY_ARROW_LEFT;
        case KEY_RIGHT: return CON_KEY_ARROW_RIGHT;
        // Linux terminals usually send '\n'; some send '\r' or ncurses KEY_ENTER
        case KEY_ENTER:
        case '\n':
        case '\r':
            return CON_KEY_ENTER;
        default:
            return c;
    }
}

int ConKbhit()
{
    nodelay(stdscr, TRUE);
    int c = getch();
    nodelay(stdscr, FALSE);
    if (c == ERR) return 0;
    ungetch(c);
    return 1;
}
