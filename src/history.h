#ifndef HISTORY_H
#define HISTORY_H

#include "game.h"

/* ── Maximum number of moves you can undo ────────────────────────────── */
#define MAX_HISTORY 256

/* ── The undo stack ───────────────────────────────────────────────────── */
typedef struct {
    GameState states[MAX_HISTORY];   /* saved snapshots of game state     */
    int       top;                    /* index of the top of the stack     */
} History;

/* ── Function declarations ────────────────────────────────────────────── */
void history_init(History *h);
void history_push(History *h, const GameState *gs);
int  history_pop(History *h, GameState *gs);

#endif /* HISTORY_H */
