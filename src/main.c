#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "types.h"

/* ── Draw one tile cell at grid position (col, row) ──────────────────── */
static void draw_tile(SDL_Renderer *ren, int col, int row, Tile tile)
{
    SDL_Rect rect = {
        .x = col * TILE_SIZE,
        .y = row * TILE_SIZE,
        .w = TILE_SIZE,
        .h = TILE_SIZE
    };

    Color c;
    switch (tile) {
        case TILE_WALL:   c = COLOR_WALL;   break;
        case TILE_FLOOR:  c = COLOR_FLOOR;  break;
        case TILE_TARGET: c = COLOR_TARGET; break;
        case TILE_BOX:    c = COLOR_BOX;    break;
        case TILE_BOX_ON: c = COLOR_BOX_ON; break;
        case TILE_PLAYER: c = COLOR_PLAYER; break;
        default:          c = COLOR_BG;     break;
    }

    /* filled tile */
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(ren, &rect);

    /* 1-px grid border so every cell is visible */
    SDL_SetRenderDrawColor(ren,
        COLOR_GRID.r, COLOR_GRID.g, COLOR_GRID.b, COLOR_GRID.a);
    SDL_RenderDrawRect(ren, &rect);
}

/* ── Temporary demo grid (replaced by level loader in next step) ──────── */
static const Tile demo_grid[GRID_ROWS][GRID_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,2,2,2,2,2,2,1},
    {1,2,1,4,2,2,3,2,2,2,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,2,6,2,2,2,5,2,2,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}
};

int main(void)
{
    /* ── SDL2 init ───────────────────────────────────────────────────── */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Sokoban",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(
        win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    /* ── Main loop ───────────────────────────────────────────────────── */
    bool running = true;
    SDL_Event e;

    while (running) {
        /* events */
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)                       running = false;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE)          running = false;
        }

        /* clear */
        SDL_SetRenderDrawColor(ren,
            COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, COLOR_BG.a);
        SDL_RenderClear(ren);

        /* draw every tile in the demo grid */
        for (int row = 0; row < GRID_ROWS; row++)
            for (int col = 0; col < GRID_COLS; col++)
                draw_tile(ren, col, row, demo_grid[row][col]);

        SDL_RenderPresent(ren);
    }

    /* ── Cleanup ─────────────────────────────────────────────────────── */
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}