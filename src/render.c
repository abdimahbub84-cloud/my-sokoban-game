#define SDL_MAIN_HANDLED
#include "render.h"
#include "types.h"
#include <SDL2/SDL.h>

/* ── Draw a filled rectangle with a given color ──────────────────────── */
static void set_color(SDL_Renderer *ren, Color c)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
}

/* ── Draw one tile at grid position (col, row) ───────────────────────── */
static void draw_tile(SDL_Renderer *ren, int col, int row, Tile tile)
{
    SDL_Rect rect = {
        col * TILE_SIZE,
        row * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };

    /* pick fill color based on tile type */
    switch (tile) {
        case TILE_WALL:
            set_color(ren, COLOR_WALL);
            SDL_RenderFillRect(ren, &rect);
            break;

        case TILE_FLOOR:
            set_color(ren, COLOR_FLOOR);
            SDL_RenderFillRect(ren, &rect);
            break;

        case TILE_TARGET:
            /* floor + a small gold diamond in the center */
            set_color(ren, COLOR_FLOOR);
            SDL_RenderFillRect(ren, &rect);
            set_color(ren, COLOR_TARGET);
            SDL_Rect dot = {
                col * TILE_SIZE + TILE_SIZE / 2 - 6,
                row * TILE_SIZE + TILE_SIZE / 2 - 6,
                12, 12
            };
            SDL_RenderFillRect(ren, &dot);
            break;

        case TILE_BOX:
            set_color(ren, COLOR_BOX);
            SDL_RenderFillRect(ren, &rect);
            /* darker border */
            set_color(ren, (Color){100, 60, 20, 255});
            SDL_RenderDrawRect(ren, &rect);
            break;

        case TILE_BOX_ON:
            /* green box = on target */
            set_color(ren, COLOR_BOX_ON);
            SDL_RenderFillRect(ren, &rect);
            set_color(ren, (Color){30, 120, 50, 255});
            SDL_RenderDrawRect(ren, &rect);
            break;

        case TILE_PLAYER:
            /* floor underneath */
            set_color(ren, COLOR_FLOOR);
            SDL_RenderFillRect(ren, &rect);
            /* player as a smaller blue square centered in the tile */
            set_color(ren, COLOR_PLAYER);
            SDL_Rect player = {
                col * TILE_SIZE + 8,
                row * TILE_SIZE + 8,
                TILE_SIZE - 16,
                TILE_SIZE - 16
            };
            SDL_RenderFillRect(ren, &player);
            break;

        default:
            /* TILE_EMPTY — just background */
            set_color(ren, COLOR_BG);
            SDL_RenderFillRect(ren, &rect);
            break;
    }

    /* subtle grid line on every tile */
    set_color(ren, COLOR_GRID);
    SDL_RenderDrawRect(ren, &rect);
}

/* ── Draw the HUD (level number + move counter) ──────────────────────── */
static void draw_hud(SDL_Renderer *ren, const GameState *gs)
{
    /* simple colored bar at the bottom */
    SDL_Rect bar = { 0, WINDOW_H - 24, WINDOW_W, 24 };
    set_color(ren, (Color){20, 20, 20, 255});
    SDL_RenderFillRect(ren, &bar);

    /* we can't use TTF without SDL_ttf, so draw pixel blocks as indicators */
    /* level indicator — small white squares on the left */
    for (int i = 0; i <= gs->level_index && i < 50; i++) {
        SDL_Rect pip = { 4 + i * 10, WINDOW_H - 18, 8, 12 };
        set_color(ren, (Color){200, 200, 200, 255});
        SDL_RenderFillRect(ren, &pip);
    }

    /* move counter — small colored squares on the right */
    int moves = gs->move_count;
    int x = WINDOW_W - 4;
    while (moves > 0 && x > WINDOW_W / 2) {
        x -= 10;
        SDL_Rect pip = { x, WINDOW_H - 18, 8, 12 };
        set_color(ren, (Color){60, 140, 220, 255});
        SDL_RenderFillRect(ren, &pip);
        moves--;
        if (moves > 50) break;   /* cap display at 50 pips */
    }

    /* if level complete — flash a green overlay */
    if (gs->completed) {
        set_color(ren, (Color){0, 200, 0, 60});
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_Rect overlay = { 0, 0, WINDOW_W, WINDOW_H };
        SDL_RenderFillRect(ren, &overlay);
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
    }
}

/* ── Main render function — called every frame ───────────────────────── */
void render_frame(SDL_Renderer *ren, const GameState *gs)
{
    /* clear screen */
    set_color(ren, COLOR_BG);
    SDL_RenderClear(ren);

    /* center the level grid in the window */
    int offset_x = (WINDOW_W - gs->cols * TILE_SIZE) / 2;
    int offset_y = (WINDOW_H - gs->rows * TILE_SIZE) / 2;

    /* draw every tile */
    for (int row = 0; row < gs->rows; row++) {
        for (int col = 0; col < gs->cols; col++) {
            Tile tile = gs->grid[row][col];

            /* check if player is here */
            if (col == gs->player.x && row == gs->player.y)
                tile = TILE_PLAYER;

            /* check if a box is here */
            for (int b = 0; b < gs->box_count; b++) {
                if (gs->boxes[b].x == col && gs->boxes[b].y == row) {
                    tile = (gs->grid[row][col] == TILE_TARGET)
                           ? TILE_BOX_ON : TILE_BOX;
                    break;
                }
            }

            SDL_Rect dest = {
                offset_x + col * TILE_SIZE,
                offset_y + row * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

            /* temporarily shift drawing origin */
            SDL_Rect save = { col * TILE_SIZE, row * TILE_SIZE,
                              TILE_SIZE, TILE_SIZE };
            (void)save;
            draw_tile(ren, offset_x / TILE_SIZE + col,
                           offset_y / TILE_SIZE + row, tile);
        }
    }

    draw_hud(ren, gs);

    SDL_RenderPresent(ren);
}
