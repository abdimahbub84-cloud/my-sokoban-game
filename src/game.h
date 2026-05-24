#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "input.h"

/* ── Maximum level dimensions ─────────────────────────────────────────── */
#define MAX_COLS 20
#define MAX_ROWS 20
#define MAX_BOXES 32

/* ── Game state ───────────────────────────────────────────────────────── */
typedef struct {
    Tile    grid[MAX_ROWS][MAX_COLS];   /* current tile map               */
    int     cols;                        /* actual level width             */
    int     rows;                        /* actual level height            */
    Vec2    player;                      /* player grid position           */
    Vec2    boxes[MAX_BOXES];            /* all box positions              */
    int     box_count;                   /* number of boxes                */
    int     move_count;                  /* moves made so far              */
    int     level_index;                 /* which level we are on          */
    int     total_levels;                /* total levels loaded            */
    int     completed;                   /* 1 if level is solved           */
} GameState;

/* ── Function declarations ────────────────────────────────────────────── */
void game_init(GameState *gs);
void game_update(GameState *gs, Action action);
int  game_is_complete(const GameState *gs);

#endif /* GAME_H */
