#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "game.h"

extern SDL_Rect g_btn_next;
extern SDL_Rect g_btn_quit;
extern SDL_Rect g_btn_play;
extern SDL_Rect g_btn_menu_quit;

void render_set_screen_size(int w, int h);
void render_frame(SDL_Renderer *ren, const GameState *gs);
void render_menu(SDL_Renderer *ren);

#endif