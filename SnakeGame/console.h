#pragma once
#include <cstdarg>

// Colors used by the UI (Win32 FOREGROUND_* values)
enum {
    CON_GREEN   = 10,
    CON_CYAN    = 11,
    CON_RED     = 12,
    CON_MAGENTA = 13,
    CON_YELLOW  = 14,
    CON_WHITE   = 15
};

// Normalized key codes from ConGetch / ConKbhit
// Named CON_KEY_* to avoid clashing with ncurses KEY_* macros.
enum {
    CON_KEY_ENTER       = 13,
    CON_KEY_ESC         = 27,
    CON_KEY_SPACE       = 32,
    CON_KEY_ARROW_LEFT  = 75,
    CON_KEY_ARROW_UP    = 72,
    CON_KEY_ARROW_RIGHT = 77,
    CON_KEY_ARROW_DOWN  = 80
};

void ConInit(int width, int height);
void ConShutdown();
void ConClear();
void ConSleep(int ms);
void ConFlush();

void ConGotoXY(int x, int y);
void ConTextColor(int color);
void ConResizeConsole(int width, int height);

int ConWriteXY(int x, int y, int color, const char* fmt, ...);

int ConGetch();   // blocking
int ConKbhit();   // 1 if key ready, else 0
