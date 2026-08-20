#include "console.h"
#include <windows.h>
#include <conio.h>
#include <cstdio>
#include <cstdarg>

enum {
    WIN_COLOR_MASK     = 0x000f,
    WIN_ATTR_CLEAR_FG  = 0xfff0,
    WIN_EXT_PREFIX_0   = 0,
    WIN_EXT_PREFIX_224 = 224
};

void ConInit(int width, int height)
{
    ConResizeConsole(width, height);
}

void ConShutdown()
{
}

void ConClear()
{
    system("cls");
}

void ConSleep(int ms)
{
    Sleep(ms);
}

void ConFlush()
{
    fflush(stdout);
}

void ConGotoXY(int x, int y)
{
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void ConTextColor(int color)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(h, &info);
    WORD attr = info.wAttributes;
    attr &= WIN_ATTR_CLEAR_FG;
    attr |= (WORD)(color & WIN_COLOR_MASK);
    SetConsoleTextAttribute(h, attr);
}

void ConResizeConsole(int width, int height)
{
    COORD coord = { (SHORT)width, (SHORT)height };
    SMALL_RECT rect = { 0, 0, (SHORT)(width - 1), (SHORT)(height - 1) };
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleScreenBufferSize(h, coord);
    SetConsoleWindowInfo(h, TRUE, &rect);
}

int ConWriteXY(int x, int y, int color, const char* fmt, ...)
{
    ConGotoXY(x, y);
    ConTextColor(color);
    va_list args;
    va_start(args, fmt);
    int n = vprintf(fmt, args);
    va_end(args);
    return n;
}

int ConGetch()
{
    int c = _getch();
    if (c == WIN_EXT_PREFIX_0 || c == WIN_EXT_PREFIX_224) {
        int ext = _getch();
        switch (ext) {
            case CON_KEY_ARROW_UP:    return CON_KEY_ARROW_UP;
            case CON_KEY_ARROW_DOWN:  return CON_KEY_ARROW_DOWN;
            case CON_KEY_ARROW_LEFT:  return CON_KEY_ARROW_LEFT;
            case CON_KEY_ARROW_RIGHT: return CON_KEY_ARROW_RIGHT;
            default:                  return ext;
        }
    }
    // Normalize CR/LF to CON_KEY_ENTER
    if (c == '\n' || c == '\r')
        return CON_KEY_ENTER;
    return c;
}

int ConKbhit()
{
    return _kbhit();
}
