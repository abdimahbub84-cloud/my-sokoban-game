#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

/* ── All possible player actions ─────────────────────────────────────── */
typedef enum {
    ACTION_NONE    = 0,
    ACTION_UP      = 1,
    ACTION_DOWN    = 2,
    ACTION_LEFT    = 3,
    ACTION_RIGHT   = 4,
    ACTION_UNDO    = 5,   /* Ctrl+Z or U key  */
    ACTION_RESTART = 6,   /* R key            */
    ACTION_QUIT    = 7    /* ESC key          */
} Action;

/* ── Read one SDL event and return the corresponding action ───────────── */
Action input_handle_event(const SDL_Event *e);

#endif /* INPUT_H */
