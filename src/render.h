#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "game.h"

/* ── Function declarations ────────────────────────────────────────────── */
void render_frame(SDL_Renderer *ren, const GameState *gs);

#endif /* RENDER_H */
