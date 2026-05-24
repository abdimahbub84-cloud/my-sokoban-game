#ifndef TYPES_H
#define TYPES_H

/* ── Window & tile dimensions ─────────────────────────────────────────── */
#define TILE_SIZE   64          /* pixels per tile (one cell in the grid)  */
#define GRID_COLS   12          /* visible columns                         */
#define GRID_ROWS   10          /* visible rows                            */

#define WINDOW_W    (TILE_SIZE * GRID_COLS)   /* 768 px                   */
#define WINDOW_H    (TILE_SIZE * GRID_ROWS)   /* 640 px                   */

/* ── Tile types ───────────────────────────────────────────────────────── */
typedef enum {
    TILE_EMPTY  = 0,   /* air / outside the warehouse                     */
    TILE_WALL   = 1,   /* solid wall, impassable                          */
    TILE_FLOOR  = 2,   /* walkable floor                                  */
    TILE_TARGET = 3,   /* goal square (box must land here)                */
    TILE_BOX    = 4,   /* pushable box (not on target)                    */
    TILE_BOX_ON = 5,   /* box sitting on a target                         */
    TILE_PLAYER = 6    /* player position                                 */
} Tile;

/* ── 2-D integer vector (grid coordinates or pixel offsets) ───────────── */
typedef struct {
    int x;
    int y;
} Vec2;

/* ── Colours used by the renderer (RGBA) ─────────────────────────────── */
typedef struct {
    unsigned char r, g, b, a;
} Color;

#define COLOR_BG        (Color){30,  30,  30,  255}   /* dark background  */
#define COLOR_WALL      (Color){80,  80,  80,  255}   /* grey wall        */
#define COLOR_FLOOR     (Color){50,  50,  50,  255}   /* dark floor       */
#define COLOR_TARGET    (Color){180, 140, 40,  255}   /* gold target dot  */
#define COLOR_BOX       (Color){160, 100, 40,  255}   /* brown box        */
#define COLOR_BOX_ON    (Color){60,  180, 80,  255}   /* green = solved   */
#define COLOR_PLAYER    (Color){60,  140, 220, 255}   /* blue player      */
#define COLOR_GRID      (Color){45,  45,  45,  255}   /* subtle grid line */

#endif /* TYPES_H */
