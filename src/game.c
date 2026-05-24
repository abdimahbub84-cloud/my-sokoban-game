#include "game.h"
#include <string.h>

/* ── Initialize a blank game state ───────────────────────────────────── */
void game_init(GameState *gs)
{
    memset(gs, 0, sizeof(GameState));
    gs->level_index = 0;
    gs->move_count  = 0;
    gs->completed   = 0;
}

/* ── Check if a tile is walkable (not a wall or empty) ───────────────── */
static int is_walkable(Tile t)
{
    return t == TILE_FLOOR || t == TILE_TARGET;
}

/* ── Check if a tile has a box on it ─────────────────────────────────── */
static int has_box(const GameState *gs, int col, int row)
{
    for (int i = 0; i < gs->box_count; i++)
        if (gs->boxes[i].x == col && gs->boxes[i].y == row)
            return i;   /* return box index */
    return -1;          /* no box here      */
}

/* ── Update the grid tile at a position ──────────────────────────────── */
static void update_tile(GameState *gs, int col, int row)
{
    Tile base = gs->grid[row][col];

    /* keep walls and empty as-is */
    if (base == TILE_WALL || base == TILE_EMPTY) return;

    int box_idx = has_box(gs, col, row);

    if (col == gs->player.x && row == gs->player.y)
        gs->grid[row][col] = TILE_PLAYER;
    else if (box_idx >= 0) {
        /* check if this box is on a target — we need the original tile */
        gs->grid[row][col] = (base == TILE_TARGET || base == TILE_BOX_ON)
                             ? TILE_BOX_ON : TILE_BOX;
    } else {
        gs->grid[row][col] = (base == TILE_TARGET || base == TILE_BOX_ON)
                             ? TILE_TARGET : TILE_FLOOR;
    }
}

/* ── Try to move the player in a direction ───────────────────────────── */
static void try_move(GameState *gs, int dx, int dy)
{
    int nx = gs->player.x + dx;   /* next player position */
    int ny = gs->player.y + dy;

    /* out of bounds check */
    if (nx < 0 || nx >= gs->cols || ny < 0 || ny >= gs->rows) return;

    Tile next = gs->grid[ny][nx];

    /* can't walk into walls or empty */
    if (next == TILE_WALL || next == TILE_EMPTY) return;

    int box_idx = has_box(gs, nx, ny);

    if (box_idx >= 0) {
        /* there is a box — try to push it */
        int bx = nx + dx;
        int by = ny + dy;

        /* box push out of bounds */
        if (bx < 0 || bx >= gs->cols || by < 0 || by >= gs->rows) return;

        Tile behind = gs->grid[by][bx];

        /* can't push into wall, empty, or another box */
        if (behind == TILE_WALL || behind == TILE_EMPTY) return;
        if (has_box(gs, bx, by) >= 0) return;

        /* move the box */
        gs->boxes[box_idx].x = bx;
        gs->boxes[box_idx].y = by;
    }

    /* move the player */
    gs->player.x = nx;
    gs->player.y = ny;
    gs->move_count++;

    /* check win condition */
    gs->completed = game_is_complete(gs);
}

/* ── Main update function called every frame with player action ───────── */
void game_update(GameState *gs, Action action)
{
    if (gs->completed) return;   /* no moves after level is solved */

    switch (action) {
        case ACTION_UP:    try_move(gs,  0, -1); break;
        case ACTION_DOWN:  try_move(gs,  0,  1); break;
        case ACTION_LEFT:  try_move(gs, -1,  0); break;
        case ACTION_RIGHT: try_move(gs,  1,  0); break;
        default: break;
    }
}

/* ── Check if all boxes are on targets ───────────────────────────────── */
int game_is_complete(const GameState *gs)
{
    for (int i = 0; i < gs->box_count; i++) {
        int bx = gs->boxes[i].x;
        int by = gs->boxes[i].y;
        if (gs->grid[by][bx] != TILE_TARGET &&
            gs->grid[by][bx] != TILE_BOX_ON)
            return 0;   /* at least one box not on target */
    }
    return 1;   /* all boxes on targets */
}