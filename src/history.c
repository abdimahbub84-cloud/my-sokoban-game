#include "history.h"
#include <string.h>

/* ── Initialize empty undo stack ─────────────────────────────────────── */
void history_init(History *h)
{
    h->top = 0;
}

/* ── Push current game state onto the stack (called before every move)── */
void history_push(History *h, const GameState *gs)
{
    if (h->top >= MAX_HISTORY) {
        /* stack full — shift everything down to make room */
        memmove(&h->states[0], &h->states[1],
                sizeof(GameState) * (MAX_HISTORY - 1));
        h->top = MAX_HISTORY - 1;
    }
    memcpy(&h->states[h->top], gs, sizeof(GameState));
    h->top++;
}

/* ── Pop last state off the stack (called on undo) ───────────────────── */
/* returns 1 if successful, 0 if nothing to undo                          */
int history_pop(History *h, GameState *gs)
{
    if (h->top <= 0)
        return 0;   /* nothing to undo */

    h->top--;
    memcpy(gs, &h->states[h->top], sizeof(GameState));
    return 1;
}
