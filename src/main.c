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

typedef enum {
    STATE_MENU,
    STATE_PLAYING
} AppState;

static int point_in_rect(int x, int y, SDL_Rect r)
{
    return x >= r.x && x <= r.x + r.w &&
           y >= r.y && y <= r.y + r.h;
}

static void load_level(GameState *gs, History *h, int index)
{
    history_init(h);
    level_apply(index, gs);
    gs->start_time = SDL_GetTicks();
    gs->elapsed_ms = 0;
}

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
    if (!win) { SDL_Quit(); return 1; }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) { SDL_DestroyWindow(win); SDL_Quit(); return 1; }

    int screen_w, screen_h;
    SDL_GetWindowSize(win, &screen_w, &screen_h);
    render_set_screen_size(screen_w, screen_h);

    GameState gs;
    History   history;

    game_init(&gs);
    history_init(&history);

    gs.total_levels = level_get_total();
    gs.level_index  = 0;

    AppState app_state = STATE_MENU;
    bool running = true;
    SDL_Event e;

    while (running) {

        /* ── Menu state ─────────────────────────────────────────────── */
        if (app_state == STATE_MENU) {
            render_menu(ren);

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) running = false;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                    running = false;

                if (e.type == SDL_MOUSEBUTTONDOWN &&
                    e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x;
                    int my = e.button.y;

                    if (point_in_rect(mx, my, g_btn_play)) {
                        load_level(&gs, &history, 0);
                        app_state = STATE_PLAYING;
                    } else if (point_in_rect(mx, my, g_btn_menu_quit)) {
                        running = false;
                    }
                }
            }
            continue;
        }

        /* ── Playing state ──────────────────────────────────────────── */
        game_tick(&gs);

        if (game_time_up(&gs))
            load_level(&gs, &history, gs.level_index);

        while (SDL_PollEvent(&e)) {

            if (e.type == SDL_MOUSEBUTTONDOWN &&
                e.button.button == SDL_BUTTON_LEFT) {
                if (gs.completed) {
                    int mx = e.button.x;
                    int my = e.button.y;
                    if (point_in_rect(mx, my, g_btn_next)) {
                        gs.level_index++;
                        if (gs.level_index >= gs.total_levels)
                            gs.level_index = 0;
                        load_level(&gs, &history, gs.level_index);
                    } else if (point_in_rect(mx, my, g_btn_quit)) {
                        running = false;
                    }
                }
            }

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
                    }
                    break;
                case ACTION_UNDO:
                    if (!gs.completed)
                        history_pop(&history, &gs);
                    break;
                case ACTION_RESTART:
                    load_level(&gs, &history, gs.level_index);
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