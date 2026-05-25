#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include "types.h"
#include "game.h"
#include "input.h"
#include "history.h"
#include "level.h"
#include "render.h"

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Sokoban",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP
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

    /* get actual fullscreen size and tell renderer */
    int screen_w, screen_h;
    SDL_GetWindowSize(win, &screen_w, &screen_h);
    render_set_screen_size(screen_w, screen_h);

    /* game setup */
    GameState gs;
    History   history;

    game_init(&gs);
    history_init(&history);

    gs.total_levels = level_get_total();
    gs.level_index  = 0;
    level_apply(gs.level_index, &gs);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            Action action = input_handle_event(&e);

            switch (action) {
                case ACTION_QUIT:
                    running = false;
                    break;

                case ACTION_UP:
                case ACTION_DOWN:
                case ACTION_LEFT:
                case ACTION_RIGHT:
                    if (!gs.completed) {
                        history_push(&history, &gs);
                        game_update(&gs, action);
                    } else {
                        /* level done — go to next */
                        gs.level_index++;
                        if (gs.level_index >= gs.total_levels)
                            gs.level_index = 0;
                        history_init(&history);
                        level_apply(gs.level_index, &gs);
                    }
                    break;

                case ACTION_UNDO:
                    history_pop(&history, &gs);
                    break;

                case ACTION_RESTART:
                    history_init(&history);
                    level_apply(gs.level_index, &gs);
                    break;

                default:
                    break;
            }
        }

        render_frame(ren, &gs);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}