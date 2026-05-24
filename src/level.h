#ifndef LEVEL_H
#define LEVEL_H

#include "types.h"
#include "game.h"

/* ── Total number of levels ───────────────────────────────────────────── */
#define TOTAL_LEVELS 50

/* ── A single level ───────────────────────────────────────────────────── */
typedef struct {
    const char *rows[MAX_ROWS];   /* each string is one row of the level  */
    int         row_count;         /* number of rows                       */
} LevelDef;

/* ── Function declarations ────────────────────────────────────────────── */
void level_apply(int index, GameState *gs);
int  level_get_total(void);

#endif /* LEVEL_H */