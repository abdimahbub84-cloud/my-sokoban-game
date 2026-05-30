#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "input.h"

#define MAX_COLS  20
#define MAX_ROWS  20
#define MAX_BOXES 32

typedef struct {
    Tile    grid[MAX_ROWS][MAX_COLS];
    int     cols;
    int     rows;
    Vec2    player;
    Vec2    boxes[MAX_BOXES];
    int     box_count;
    int     move_count;
    int     level_index;
    int     total_levels;
    int     completed;
    Uint32  start_time;    /* SDL tick when level started  */
    Uint32  elapsed_ms;    /* milliseconds spent on level  */
    int     facing;        /* 0=right 1=left 2=up 3=down   */
    int     is_moving;     /* 1 if moved this frame         */
    Uint32  anim_tick;     /* frame counter for animation   */
} GameState;

void game_init(GameState *gs);
void game_update(GameState *gs, Action action);
int  game_is_complete(const GameState *gs);
void game_tick(GameState *gs);
int  game_time_up(const GameState *gs);   /* call every frame to update timer */

#endif