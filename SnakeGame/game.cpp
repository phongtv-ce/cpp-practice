#include "game.h"
#include "map_data.h"

#include <cstdlib>
#include <cstring>

static const int kDx[DIR_COUNT] = { 1,  0, -1, 0 };
static const int kDy[DIR_COUNT] = { 0, -1,  0, 1 };

static bool is_free(const Game& g, int x, int y)
{
    if (y < 0 || y >= MAP_ROWS || x < 0 || x >= MAP_COLS)
        return false;
    char c = g.map[y][x];
    return c == CELL_EMPTY || c == CELL_DECOR;
}

static void spawn_prey(Game& g)
{
    const int max_random_tries = PLAY_W * PLAY_H;

    for (int i = 0; i < max_random_tries; i++) {
        g.prey.x = rand() % PLAY_W + 1;
        g.prey.y = rand() % PLAY_H + 1;
        if (is_free(g, g.prey.x, g.prey.y))
            return;
    }

    for (int y = 1; y <= PLAY_H; y++) {
        for (int x = 1; x <= PLAY_W; x++) {
            if (is_free(g, x, y)) {
                g.prey.x = x;
                g.prey.y = y;
                return;
            }
        }
    }

    g.prey.x = -1;
    g.prey.y = -1;
}

void game_init(Game& g, int speed_ms)
{
    for (int y = 0; y < MAP_ROWS; y++) {
        std::memset(g.map[y], CELL_EMPTY, MAP_COLS);
        std::strncpy(g.map[y], kBackdropRows[y], MAP_COLS);
    }

    g.body[0] = { START_X, START_Y };
    g.len     = 1;
    g.map[START_Y][START_X] = CELL_SNAKE;

    g.dir      = DIR_RIGHT;
    g.score    = 0;
    g.level    = 0;
    g.speed_ms = speed_ms;
    g.alive    = true;

    spawn_prey(g);
}

void game_set_dir(Game& g, Dir d)
{
    if (!g.alive) return;
    // Block reverse: Right <-> Left, Up <-> Down
    if ((g.dir + 2) % DIR_COUNT == d) return;
    g.dir = d;
}

StepResult game_step(Game& g)
{
    if (!g.alive) return STEP_DIED;

    Point old_tail = g.body[g.len - 1];

    for (int i = g.len; i > 0; i--)
        g.body[i] = g.body[i - 1];

    g.body[0].x += kDx[g.dir];
    g.body[0].y += kDy[g.dir];

    Point head = g.body[0];
    bool ate  = (head.x == g.prey.x && head.y == g.prey.y);
    bool grow = ate && g.len < SNAKE_MAX;

    if (!grow)
        g.map[old_tail.y][old_tail.x] = CELL_EMPTY;

    char cell = g.map[head.y][head.x];
    if (cell != CELL_EMPTY && cell != CELL_DECOR) {
        g.alive = false;
        return STEP_DIED;
    }

    g.map[head.y][head.x] = CELL_SNAKE;

    if (ate) {
        g.score++;
        if (grow) g.len++;
        if (g.score % SCORE_PER_LEVEL == 0) {
            g.level++;
            if (g.speed_ms > SPEED_LEVEL_FLOOR)
                g.speed_ms -= SPEED_ON_LEVEL;
        }
        spawn_prey(g);
        if (g.prey.x < 0)
            g.alive = false;
        return STEP_ATE;
    }

    return STEP_OK;
}

void game_speed_up(Game& g)
{
    if (g.speed_ms > SPEED_MIN)
        g.speed_ms -= SPEED_STEP;
}

void game_speed_down(Game& g)
{
    if (g.speed_ms < SPEED_MAX)
        g.speed_ms += SPEED_STEP;
}

int game_speed_level(const Game& g)
{
    return (SPEED_MAX - g.speed_ms) / SPEED_STEP;
}
