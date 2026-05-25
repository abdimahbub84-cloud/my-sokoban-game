#define SDL_MAIN_HANDLED
#include "render.h"
#include "types.h"
#include <SDL2/SDL.h>

static int SCREEN_W = 768;
static int SCREEN_H = 640;

void render_set_screen_size(int w, int h)
{
    SCREEN_W = w;
    SCREEN_H = h;
}

static void set_color(SDL_Renderer *ren, Color c)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
}

static void fill_rect(SDL_Renderer *ren, int x, int y, int w, int h, Color c)
{
    set_color(ren, c);
    SDL_Rect r = { x, y, w, h };
    SDL_RenderFillRect(ren, &r);
}

static void fill_circle(SDL_Renderer *ren, int cx, int cy, int radius, Color c)
{
    set_color(ren, c);
    for (int dy = -radius; dy <= radius; dy++) {
        int dx = (int)SDL_sqrt((double)(radius * radius - dy * dy));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

/* ── Fixed tile size that fills the whole screen ─────────────────────── */
static int get_tile_size(const GameState *gs)
{
    int hud = 44;
    int tw = SCREEN_W / (gs->cols > 0 ? gs->cols : 1);
    int th = (SCREEN_H - hud) / (gs->rows > 0 ? gs->rows : 1);
    return tw < th ? tw : th;
}

static void draw_floor(SDL_Renderer *ren, int px, int py, int ts)
{
    fill_rect(ren, px, py, ts, ts, (Color){50, 50, 50, 255});
    set_color(ren, (Color){65, 65, 65, 255});
    SDL_Rect r = { px, py, ts, ts };
    SDL_RenderDrawRect(ren, &r);
}

static void draw_wall(SDL_Renderer *ren, int px, int py, int ts)
{
    fill_rect(ren, px, py, ts, ts, (Color){90, 80, 70, 255});
    set_color(ren, (Color){120, 110, 100, 255});
    SDL_RenderDrawLine(ren, px, py, px + ts - 1, py);
    SDL_RenderDrawLine(ren, px, py, px, py + ts - 1);
    set_color(ren, (Color){60, 55, 50, 255});
    SDL_RenderDrawLine(ren, px+ts-1, py, px+ts-1, py+ts-1);
    SDL_RenderDrawLine(ren, px, py+ts-1, px+ts-1, py+ts-1);
}

static void draw_target(SDL_Renderer *ren, int px, int py, int ts)
{
    draw_floor(ren, px, py, ts);
    int cx = px + ts/2;
    int cy = py + ts/2;
    int r  = ts/3;
    fill_circle(ren, cx, cy, r,     (Color){0, 200,  0, 255});
    fill_circle(ren, cx, cy, r*2/3, (Color){0, 100,  0, 255});
    fill_circle(ren, cx, cy, r/3,   (Color){0, 240,  0, 255});
}

static void draw_spongebob(SDL_Renderer *ren, int px, int py, int ts, int on_target)
{
    int x  = px + ts/10;
    int y  = py + ts/10;
    int s  = ts - ts/5;
    int cx = px + ts/2;

    if (on_target)
        fill_rect(ren, px, py, ts, ts, (Color){0, 220, 0, 255});

    fill_rect(ren, x, y, s, s, (Color){255, 220, 50, 255});

    Color hole = {200, 160, 20, 255};
    int hs = ts/16 + 1;
    fill_circle(ren, cx - ts/8,  py + ts/5,      hs+1, hole);
    fill_circle(ren, cx + ts/10, py + ts/5 - 2,  hs,   hole);
    fill_circle(ren, cx - ts/16, py + ts*2/5,     hs,   hole);
    fill_circle(ren, cx + ts/6,  py + ts*2/5 + 2, hs+1, hole);
    fill_circle(ren, cx - ts/6,  py + ts*3/5,     hs,   hole);
    fill_circle(ren, cx + ts/14, py + ts*3/5 + 2, hs,   hole);

    int er = ts/9;
    fill_circle(ren, cx - ts/7, py + ts/4, er+1, (Color){255,255,255,255});
    fill_circle(ren, cx + ts/7, py + ts/4, er+1, (Color){255,255,255,255});
    fill_circle(ren, cx - ts/7, py + ts/4, er-1, (Color){100,180,255,255});
    fill_circle(ren, cx + ts/7, py + ts/4, er-1, (Color){100,180,255,255});
    fill_circle(ren, cx - ts/8, py + ts/4, er/2, (Color){0,0,0,255});
    fill_circle(ren, cx + ts/6, py + ts/4, er/2, (Color){0,0,0,255});

    fill_rect(ren, cx - ts/16, py + ts*2/5 - ts/16, ts/8, ts/7, (Color){230,170,30,255});

    set_color(ren, (Color){80,40,0,255});
    for (int i = -ts/7; i <= ts/7; i++) {
        int yoff = (i*i)/(ts/7);
        SDL_RenderDrawPoint(ren, cx+i, py + ts*3/5 + yoff);
        SDL_RenderDrawPoint(ren, cx+i, py + ts*3/5 + yoff + 1);
    }

    int tw2 = ts/9;
    fill_rect(ren, cx-tw2-1, py+ts*3/5, tw2, ts/10, (Color){255,255,255,255});
    fill_rect(ren, cx+1,     py+ts*3/5, tw2, ts/10, (Color){255,255,255,255});
    set_color(ren, (Color){80,40,0,255});
    SDL_RenderDrawLine(ren, cx, py+ts*3/5, cx, py+ts*3/5+ts/10);

    fill_rect(ren, x, py+s-ts/8, s, ts/12, (Color){100,70,30,255});
    fill_rect(ren, cx-ts/14, py+s-ts/7, ts/7, ts/7, (Color){180,150,0,255});

    set_color(ren, (Color){180,140,0,255});
    SDL_Rect border = { x, y, s, s };
    SDL_RenderDrawRect(ren, &border);
}

static void draw_cat(SDL_Renderer *ren, int px, int py, int ts)
{
    Color orange = {220,140, 50,255};
    Color dark   = { 80, 40, 10,255};
    Color white  = {255,255,255,255};
    Color green  = { 80,200, 80,255};
    Color pink   = {255,160,180,255};
    Color black  = {  0,  0,  0,255};

    int cx = px + ts/2;
    int cy = py + ts/2;
    int br = ts/4;
    int hr = ts*9/32;

    fill_circle(ren, cx, cy + br/2, br,  orange);
    fill_circle(ren, cx, cy - br/4, hr,  orange);

    fill_rect(ren, cx-hr-2, cy-hr,   hr/2+2, hr/2+2, orange);
    fill_rect(ren, cx-hr,   cy-hr-2, hr/2-2, hr/2-2, pink);
    fill_rect(ren, cx+hr/2, cy-hr,   hr/2+2, hr/2+2, orange);
    fill_rect(ren, cx+hr/2+2, cy-hr-2, hr/2-2, hr/2-2, pink);

    int er = ts/14;
    fill_circle(ren, cx-hr/2, cy-hr/3, er+1, white);
    fill_circle(ren, cx+hr/2, cy-hr/3, er+1, white);
    fill_circle(ren, cx-hr/2, cy-hr/3, er-1, green);
    fill_circle(ren, cx+hr/2, cy-hr/3, er-1, green);
    fill_circle(ren, cx-hr/2, cy-hr/3, er/2, black);
    fill_circle(ren, cx+hr/2, cy-hr/3, er/2, black);

    fill_rect(ren, cx-2, cy-1, 5, 4, pink);

    set_color(ren, dark);
    SDL_RenderDrawLine(ren, cx-2, cy,   cx-hr-4, cy-2);
    SDL_RenderDrawLine(ren, cx-2, cy+2, cx-hr-4, cy+4);
    SDL_RenderDrawLine(ren, cx+2, cy,   cx+hr+4, cy-2);
    SDL_RenderDrawLine(ren, cx+2, cy+2, cx+hr+4, cy+4);

    set_color(ren, orange);
    SDL_RenderDrawLine(ren, cx+hr, cy+br/2,   cx+hr+10, cy);
    SDL_RenderDrawLine(ren, cx+hr+1, cy+br/2, cx+hr+11, cy);

    set_color(ren, dark);
    SDL_RenderDrawLine(ren, cx-hr/2, cy-hr, cx-hr/2+2, cy-hr/2);
    SDL_RenderDrawLine(ren, cx,      cy-hr-2, cx, cy-hr/2);
    SDL_RenderDrawLine(ren, cx+hr/2, cy-hr, cx+hr/2-2, cy-hr/2);
}

static void draw_hud(SDL_Renderer *ren, const GameState *gs)
{
    fill_rect(ren, 0, SCREEN_H - 44, SCREEN_W, 44, (Color){15,15,15,255});

    /* LEVEL label */
    fill_rect(ren, 10, SCREEN_H - 36, 70, 26, (Color){40,40,40,255});

    /* level number digit */
    static const unsigned char digits[10][7] = {
        {0x7,0x5,0x5,0x5,0x5,0x5,0x7},
        {0x2,0x6,0x2,0x2,0x2,0x2,0x7},
        {0x7,0x1,0x1,0x7,0x4,0x4,0x7},
        {0x7,0x1,0x1,0x7,0x1,0x1,0x7},
        {0x5,0x5,0x5,0x7,0x1,0x1,0x1},
        {0x7,0x4,0x4,0x7,0x1,0x1,0x7},
        {0x7,0x4,0x4,0x7,0x5,0x5,0x7},
        {0x7,0x1,0x1,0x1,0x1,0x1,0x1},
        {0x7,0x5,0x5,0x7,0x5,0x5,0x7},
        {0x7,0x5,0x5,0x7,0x1,0x1,0x7},
    };

    int num  = gs->level_index + 1;
    int tens = num / 10;
    int ones = num % 10;
    int bx   = 18;
    int by   = SCREEN_H - 34;
    int bs   = 3;

    if (tens > 0) {
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 3; col++)
                if (digits[tens][row] & (4 >> col))
                    fill_rect(ren, bx+col*(bs+1), by+row*(bs+1), bs, bs,
                              (Color){255,220,0,255});
        bx += 16;
    }
    for (int row = 0; row < 7; row++)
        for (int col = 0; col < 3; col++)
            if (digits[ones][row] & (4 >> col))
                fill_rect(ren, bx+col*(bs+1), by+row*(bs+1), bs, bs,
                          (Color){255,220,0,255});

    /* progress dots */
    for (int i = 0; i < gs->total_levels && i < 50; i++) {
        Color c = (i == gs->level_index) ? (Color){255,220,  0,255}
                : (i <  gs->level_index) ? (Color){ 80,160, 80,255}
                :                          (Color){ 50, 50, 50,255};
        fill_rect(ren, 90 + i*13, SCREEN_H-34, 11, 22, c);
    }

    /* move counter on right */
    int moves = gs->move_count > 50 ? 50 : gs->move_count;
    for (int i = 0; i < moves; i++)
        fill_rect(ren, SCREEN_W-8-i*10, SCREEN_H-34, 8, 22,
                  (Color){60,140,220,255});
}

static void draw_complete(SDL_Renderer *ren)
{
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    fill_rect(ren, 0, 0, SCREEN_W, SCREEN_H, (Color){0,180,0,80});
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    int bw=400, bh=120;
    fill_rect(ren, SCREEN_W/2-bw/2, SCREEN_H/2-bh/2, bw, bh, (Color){20,20,20,255});
    fill_rect(ren, SCREEN_W/2-180,  SCREEN_H/2-30,   360, 30, (Color){255,200,0,255});
    fill_rect(ren, SCREEN_W/2-180,  SCREEN_H/2+15,   360, 18, (Color){180,180,180,255});
    fill_rect(ren, SCREEN_W/2-180,  SCREEN_H/2+42,   360, 18, (Color){60,140,220,255});
}

void render_frame(SDL_Renderer *ren, const GameState *gs)
{
    set_color(ren, COLOR_BG);
    SDL_RenderClear(ren);

    int hud_h = 44;
    int ts    = get_tile_size(gs);

    /* fill entire screen with tiles — no gaps */
    int total_cols = SCREEN_W / ts;
    int total_rows = (SCREEN_H - hud_h) / ts;

    /* offset so level is centered */
    int off_x = (SCREEN_W - gs->cols * ts) / 2;
    int off_y = ((SCREEN_H - hud_h) - gs->rows * ts) / 2;

    /* fill background with floor tiles edge to edge */
    for (int row = 0; row < total_rows + 1; row++)
        for (int col = 0; col < total_cols + 1; col++)
            draw_floor(ren, col * ts, row * ts, ts);

    /* draw actual level tiles on top */
    for (int row = 0; row < gs->rows; row++) {
        for (int col = 0; col < gs->cols; col++) {
            int  px = off_x + col * ts;
            int  py = off_y + row * ts;
            Tile t  = gs->grid[row][col];
            if      (t == TILE_WALL)   draw_wall(ren, px, py, ts);
            else if (t == TILE_FLOOR)  draw_floor(ren, px, py, ts);
            else if (t == TILE_TARGET) draw_target(ren, px, py, ts);
            /* TILE_EMPTY stays as background floor */
        }
    }

    /* boxes */
    for (int b = 0; b < gs->box_count; b++) {
        int col = gs->boxes[b].x;
        int row = gs->boxes[b].y;
        int on  = (gs->grid[row][col] == TILE_TARGET);
        draw_spongebob(ren, off_x + col*ts, off_y + row*ts, ts, on);
    }

    /* cat */
    draw_cat(ren, off_x + gs->player.x*ts, off_y + gs->player.y*ts, ts);

    draw_hud(ren, gs);

    if (gs->completed)
        draw_complete(ren);

    SDL_RenderPresent(ren);
}