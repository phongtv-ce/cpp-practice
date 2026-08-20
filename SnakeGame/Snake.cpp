#include "console.h"
#include "game.h"

#include <cstdlib>
#include <ctime>

// ---------------------------------------------------------------------------
// Display characters (ASCII — works on Windows + Linux/macOS)
// ---------------------------------------------------------------------------
static const char SNAKE_CH = 'O';
static const char PREY_CH  = '$';
static const char WALL_CH  = '#';   // border & obstacles (was CP437 219)
static const char BAR_CH   = '=';   // difficulty meter
static const char DEAD_CH  = 'X';
static const char EMPTY_CH = ' ';
static const char BODY_CH  = 'o';

enum {
    HUD_ROW      = 27,
    HUD_HINT_X   = 1,
    HUD_LEVEL_X  = 50,
    HUD_SCORE_X  = 65,
    HUD_PAUSE_X  = 30,

    MENU_TITLE_X = 32,
    MENU_TITLE_Y = 1,
    MENU_ITEM_X  = 35,
    MENU_ITEM_Y0 = 4,
    MENU_HINT_X  = 20,

    DIFF_TITLE_X = 32,
    DIFF_TITLE_Y = 1,
    DIFF_HINT_X  = 20,
    DIFF_HINT_Y  = 10,
    DIFF_BAR_X   = 21,
    DIFF_BAR_Y   = 5,
    DIFF_LABEL_X = 15,

    GAMEOVER_X   = 22,
    GAMEOVER_Y   = 12,
    START_HINT_Y = HUD_ROW,

    MENU_PLAY    = 0,
    MENU_DIFF    = 1,
    MENU_QUIT    = 2,
    MENU_COUNT   = 3
};

// ---------------------------------------------------------------------------
// Draw helpers
// ---------------------------------------------------------------------------
static void draw_box(int x1, int y1, int x2, int y2, int color)
{
    for (int i = x1; i <= x2; i++)
        ConWriteXY(i, y1, color, "%c", WALL_CH);
    for (int y = y1 + 1; y < y2; y++) {
        ConWriteXY(x1, y, color, "%c", WALL_CH);
        ConWriteXY(x2, y, color, "%c", WALL_CH);
    }
    for (int i = x1; i <= x2; i++)
        ConWriteXY(i, y2, color, "%c", WALL_CH);
}

static void draw_map(const Game& g)
{
    for (int y = 0; y < MAP_ROWS; y++)
        for (int x = 0; x < MAP_COLS; x++) {
            char c = g.map[y][x];
            if (c == CELL_OBSTACLE || c == CELL_WALL)
                ConWriteXY(x, y, CON_WHITE, "%c", WALL_CH);
        }
}

static void draw_hud(const Game& g)
{
    ConWriteXY(HUD_SCORE_X, HUD_ROW, CON_GREEN, " Score: %5d ", g.score);
    ConWriteXY(HUD_LEVEL_X, HUD_ROW, CON_GREEN, " Level: %2d  ", g.level);
    ConWriteXY(HUD_HINT_X,  HUD_ROW, CON_CYAN,
               "Menu: ESC  Pause: SPACE  Speed: %2d/%d",
               game_speed_level(g), SPEED_LEVEL_MAX);
}

static void draw_snake(const Game& g)
{
    for (int i = g.len - 1; i > 0; i--)
        ConWriteXY(g.body[i].x, g.body[i].y, CON_CYAN, "%c", BODY_CH);
    ConWriteXY(g.body[0].x, g.body[0].y, CON_MAGENTA, "%c", SNAKE_CH);
}

static void draw_prey(const Game& g)
{
    if (g.prey.x >= 0 && g.prey.y >= 0)
        ConWriteXY(g.prey.x, g.prey.y, CON_GREEN, "%c", PREY_CH);
}

static Dir key_to_dir(int key)
{
    switch (key) {
        case CON_KEY_ARROW_RIGHT: return DIR_RIGHT;
        case CON_KEY_ARROW_UP:    return DIR_UP;
        case CON_KEY_ARROW_LEFT:  return DIR_LEFT;
        case CON_KEY_ARROW_DOWN:  return DIR_DOWN;
        default:                  return DIR_RIGHT;
    }
}

static bool is_arrow(int key)
{
    return key == CON_KEY_ARROW_UP || key == CON_KEY_ARROW_DOWN ||
           key == CON_KEY_ARROW_LEFT || key == CON_KEY_ARROW_RIGHT;
}

// ---------------------------------------------------------------------------
// Screens
// ---------------------------------------------------------------------------
static void screen_difficulty(Game& g)
{
    ConClear();
    ConWriteXY(DIFF_TITLE_X, DIFF_TITLE_Y, CON_GREEN, "CHON MUC DO KHO");
    ConWriteXY(DIFF_HINT_X,  DIFF_HINT_Y,  CON_RED,
               "Nhan < hoac > de chinh. Nhan ESC de ve.");
    ConFlush();

    int key;
    do {
        int lv = game_speed_level(g);
        ConWriteXY(DIFF_LABEL_X, DIFF_BAR_Y, CON_YELLOW,
                   "Muc: [%20s] %2d/%d", "", lv, SPEED_LEVEL_MAX);
        for (int i = 0; i < SPEED_LEVEL_MAX; i++)
            ConWriteXY(DIFF_BAR_X + i, DIFF_BAR_Y, CON_YELLOW,
                       "%c", i < lv ? BAR_CH : EMPTY_CH);
        ConFlush();

        key = ConGetch();
        if (key == CON_KEY_ARROW_RIGHT) game_speed_up(g);
        if (key == CON_KEY_ARROW_LEFT)  game_speed_down(g);
    } while (key != CON_KEY_ESC);
}

static void screen_play(Game& g)
{
    ConClear();
    ConResizeConsole(CONSOLE_W, CONSOLE_H);
    draw_map(g);
    draw_box(0, 0, PLAY_W + 1, PLAY_H + 1, CON_YELLOW);
    draw_snake(g);
    draw_prey(g);
    ConWriteXY(HUD_HINT_X, START_HINT_Y, CON_YELLOW, "Nhan mui ten de bat dau...");
    ConFlush();

    int key;
    do { key = ConGetch(); } while (!is_arrow(key) && key != CON_KEY_ESC);
    if (key == CON_KEY_ESC) return;
    game_set_dir(g, key_to_dir(key));
    draw_hud(g);
    ConFlush();

    while (g.alive) {
        if (ConKbhit()) {
            key = ConGetch();
            if (is_arrow(key)) {
                game_set_dir(g, key_to_dir(key));
            } else if (key == CON_KEY_SPACE) {
                ConWriteXY(HUD_PAUSE_X, HUD_ROW, CON_YELLOW, "[PAUSE]");
                while (ConGetch() != CON_KEY_SPACE) {}
                ConWriteXY(HUD_PAUSE_X, HUD_ROW, CON_CYAN, "       ");
            } else if (key == CON_KEY_ESC) {
                return;
            }
        }

        Point old_tail = g.body[g.len - 1];
        int old_len = g.len;
        StepResult result = game_step(g);

        if (result == STEP_DIED) {
            ConWriteXY(g.body[0].x, g.body[0].y, CON_RED, "%c", DEAD_CH);
            ConWriteXY(GAMEOVER_X, GAMEOVER_Y, CON_RED,
                       "GAME OVER! Press ESC to return.");
            ConFlush();
            while (ConGetch() != CON_KEY_ESC) {}
            return;
        }

        if (g.len == old_len)
            ConWriteXY(old_tail.x, old_tail.y, CON_CYAN, "%c", EMPTY_CH);
        draw_snake(g);
        draw_prey(g);
        draw_hud(g);
        ConFlush();
        ConSleep(g.speed_ms);
    }
}

// ---------------------------------------------------------------------------
int main()
{
    srand((unsigned)time(0));
    ConInit(CONSOLE_W, CONSOLE_H);

    Game g;
    game_init(g);

    static const char* kMenuItems[MENU_COUNT] = {
        "Bat dau choi",
        "Chon do kho",
        "Thoat"
    };
    int select = MENU_PLAY;

    for (;;) {
        ConClear();
        ConWriteXY(MENU_TITLE_X, MENU_TITLE_Y, CON_GREEN, "TRO CHOI CON RAN");
        for (int i = 0; i < MENU_COUNT; i++)
            ConWriteXY(MENU_ITEM_X, MENU_ITEM_Y0 + i,
                       i == select ? CON_YELLOW : CON_WHITE,
                       "%s%s", i == select ? "> " : "  ", kMenuItems[i]);
        ConWriteXY(MENU_HINT_X, MENU_ITEM_Y0 + MENU_COUNT + 2, CON_CYAN,
                   "Mui ten len/xuong, Enter xac nhan, ESC thoat.");
        ConFlush();

        int key = ConGetch();
        if (key == CON_KEY_ARROW_UP)
            select = (select - 1 + MENU_COUNT) % MENU_COUNT;
        if (key == CON_KEY_ARROW_DOWN)
            select = (select + 1) % MENU_COUNT;

        if (key == CON_KEY_ENTER) {
            if (select == MENU_PLAY) {
                game_init(g, g.speed_ms);
                screen_play(g);
            } else if (select == MENU_DIFF) {
                screen_difficulty(g);
            } else {
                break;
            }
        }
        if (key == CON_KEY_ESC) break;
    }

    ConShutdown();
    return 0;
}
