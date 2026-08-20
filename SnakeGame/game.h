#pragma once

// Pure game logic — no console / platform code.

enum Dir {
    DIR_RIGHT = 0,
    DIR_UP    = 1,
    DIR_LEFT  = 2,
    DIR_DOWN  = 3,
    DIR_COUNT = 4
};

enum StepResult {
    STEP_OK,     // moved
    STEP_ATE,    // moved and ate prey
    STEP_DIED    // hit wall / obstacle / self
};

struct Point {
    int x, y;
};

// Map cell markers
enum {
    CELL_EMPTY    = ' ',
    CELL_WALL     = '*',
    CELL_OBSTACLE = '8',
    CELL_DECOR    = '2',  // passable decoration
    CELL_SNAKE    = 'S'
};

enum {
    MAP_ROWS         = 27,
    PLAY_W           = 80,
    PLAY_H           = 25,
    MAP_COLS         = PLAY_W + 2,
    CONSOLE_W        = PLAY_W + 2,
    CONSOLE_H        = PLAY_H + 3,

    SNAKE_MAX        = 100,
    START_X          = 1,
    START_Y          = 1,

    SPEED_MIN        = 10,   // fastest (slider)
    SPEED_MAX        = 210,  // slowest (default)
    SPEED_STEP       = 10,
    SPEED_LEVEL_MAX  = (SPEED_MAX - SPEED_MIN) / SPEED_STEP,  // 20

    SCORE_PER_LEVEL  = 5,
    SPEED_ON_LEVEL   = 20,   // ms removed each level-up
    SPEED_LEVEL_FLOOR = 30   // do not auto-speed below this
};

struct Game {
    char   map[MAP_ROWS][MAP_COLS];
    Point  body[SNAKE_MAX + 1];
    int    len;
    Point  prey;
    Dir    dir;
    int    score;
    int    level;
    int    speed_ms;   // delay between ticks (lower = faster)
    bool   alive;
};

void game_init(Game& g, int speed_ms = SPEED_MAX);
void game_set_dir(Game& g, Dir d);
StepResult game_step(Game& g);

void game_speed_up(Game& g);
void game_speed_down(Game& g);
int  game_speed_level(const Game& g);  // 0 .. SPEED_LEVEL_MAX
