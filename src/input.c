#include "input.h"

/* ── Translate an SDL_Event into a game Action ────────────────────────── */
Action input_handle_event(const SDL_Event *e)
{
    if (e->type == SDL_QUIT)
        return ACTION_QUIT;

    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode key  = e->key.keysym.sym;
        SDL_Keymod  mods = SDL_GetModState();

        switch (key) {
            /* movement — arrow keys or WASD */
            case SDLK_UP:    case SDLK_w: return ACTION_UP;
            case SDLK_DOWN:  case SDLK_s: return ACTION_DOWN;
            case SDLK_LEFT:  case SDLK_a: return ACTION_LEFT;
            case SDLK_RIGHT: case SDLK_d: return ACTION_RIGHT;

            /* undo — U key or Ctrl+Z */
            case SDLK_u: return ACTION_UNDO;
            case SDLK_z:
                if (mods & KMOD_CTRL) return ACTION_UNDO;
                break;

            /* restart current level */
            case SDLK_r: return ACTION_RESTART;

            /* quit */
            case SDLK_ESCAPE: return ACTION_QUIT;

            default: break;
        }
    }

    return ACTION_NONE;
}
