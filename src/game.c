#include "game.h"
#include <string.h>
#include <SDL2/SDL.h>

void game_init(GameState *gs)
{
    memset(gs, 0, sizeof(GameState));
    gs->level_index = 0;
    gs->move_count  = 0;
    gs->completed   = 0;
    gs->start_time  = SDL_GetTicks();
    gs->elapsed_ms  = 0;
}

static int is_walkable(Tile t)
{
    return t == TILE_FLOOR || t == TILE_TARGET;
}

static int has_box(const GameState *gs, int col, int row)
{
    for (int i = 0; i < gs->box_count; i++)
        if (gs->boxes[i].x == col && gs->boxes[i].y == row)
            return i;
    return -1;
}

static void try_move(GameState *gs, int dx, int dy)
{
    int nx = gs->player.x + dx;
    int ny = gs->player.y + dy;

    if (nx < 0 || nx >= gs->cols || ny < 0 || ny >= gs->rows) return;

    Tile next = gs->grid[ny][nx];
    if (next == TILE_WALL || next == TILE_EMPTY) return;

    int box_idx = has_box(gs, nx, ny);
    if (box_idx >= 0) {
        int bx = nx + dx;
        int by = ny + dy;

        if (bx < 0 || bx >= gs->cols || by < 0 || by >= gs->rows) return;

        Tile behind = gs->grid[by][bx];
        if (behind == TILE_WALL || behind == TILE_EMPTY) return;
        if (has_box(gs, bx, by) >= 0) return;

        gs->boxes[box_idx].x = bx;
        gs->boxes[box_idx].y = by;
    }

    gs->player.x = nx;
    gs->player.y = ny;
    gs->move_count++;
    gs->completed = game_is_complete(gs);
}

void game_update(GameState *gs, Action action)
{
    if (gs->completed) return;

    switch (action) {
        case ACTION_UP:    try_move(gs,  0, -1); break;
        case ACTION_DOWN:  try_move(gs,  0,  1); break;
        case ACTION_LEFT:  try_move(gs, -1,  0); break;
        case ACTION_RIGHT: try_move(gs,  1,  0); break;
        default: break;
    }
}

int game_is_complete(const GameState *gs)
{
    for (int i = 0; i < gs->box_count; i++) {
        int bx = gs->boxes[i].x;
        int by = gs->boxes[i].y;
        if (gs->grid[by][bx] != TILE_TARGET &&
            gs->grid[by][bx] != TILE_BOX_ON)
            return 0;
    }
    return 1;
}

/* ── Update timer every frame, returns 1 if time ran out ────────────── */
void game_tick(GameState *gs)
{
    if (!gs->completed) {
        gs->elapsed_ms = SDL_GetTicks() - gs->start_time;
        if (gs->elapsed_ms >= 60000)
            gs->elapsed_ms = 60000;   /* cap at 60s */
    }
}

int game_time_up(const GameState *gs)
{
    return (!gs->completed && gs->elapsed_ms >= 60000);
}