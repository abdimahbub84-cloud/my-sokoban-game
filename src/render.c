#define SDL_MAIN_HANDLED
#include "render.h"
#include "types.h"
#include <SDL2/SDL.h>

/* ── Global game state pointer for animation ─────────────────────────── */
const GameState *g_gs_ptr = NULL;

static int SCREEN_W = 768;
static int SCREEN_H = 640;

void render_set_screen_size(int w, int h)
{
    SCREEN_W = w;
    SCREEN_H = h;
}

/* ── Button rectangles (set during draw, read by main for click detection) */
SDL_Rect g_btn_next = {0};
SDL_Rect g_btn_quit = {0};

static void set_color(SDL_Renderer *ren, Color c)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
}

static void fill_rect(SDL_Renderer *ren, int rx, int ry, int rw, int rh, Color c)
{
    set_color(ren, c);
    SDL_Rect r = { rx, ry, rw, rh };
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
    SDL_RenderDrawLine(ren, px, py, px+ts-1, py);
    SDL_RenderDrawLine(ren, px, py, px, py+ts-1);
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
    fill_circle(ren, cx-ts/8,  py+ts/5,      hs+1, hole);
    fill_circle(ren, cx+ts/10, py+ts/5-2,    hs,   hole);
    fill_circle(ren, cx-ts/16, py+ts*2/5,    hs,   hole);
    fill_circle(ren, cx+ts/6,  py+ts*2/5+2,  hs+1, hole);
    fill_circle(ren, cx-ts/6,  py+ts*3/5,    hs,   hole);
    fill_circle(ren, cx+ts/14, py+ts*3/5+2,  hs,   hole);

    int er = ts/9;
    fill_circle(ren, cx-ts/7, py+ts/4, er+1, (Color){255,255,255,255});
    fill_circle(ren, cx+ts/7, py+ts/4, er+1, (Color){255,255,255,255});
    fill_circle(ren, cx-ts/7, py+ts/4, er-1, (Color){100,180,255,255});
    fill_circle(ren, cx+ts/7, py+ts/4, er-1, (Color){100,180,255,255});
    fill_circle(ren, cx-ts/8, py+ts/4, er/2, (Color){0,0,0,255});
    fill_circle(ren, cx+ts/6, py+ts/4, er/2, (Color){0,0,0,255});

    fill_rect(ren, cx-ts/16, py+ts*2/5-ts/16, ts/8, ts/7, (Color){230,170,30,255});

    set_color(ren, (Color){80,40,0,255});
    for (int i = -ts/7; i <= ts/7; i++) {
        int yoff = (i*i)/(ts/7);
        SDL_RenderDrawPoint(ren, cx+i, py+ts*3/5+yoff);
        SDL_RenderDrawPoint(ren, cx+i, py+ts*3/5+yoff+1);
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
    /* get animation state from global game state */
    extern const GameState *g_gs_ptr;
    int facing    = g_gs_ptr ? g_gs_ptr->facing    : 0;
    int is_moving = g_gs_ptr ? g_gs_ptr->is_moving  : 0;
    Uint32 tick   = g_gs_ptr ? g_gs_ptr->anim_tick  : 0;
    Uint32 idle   = g_gs_ptr ? SDL_GetTicks()        : 0;

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

    /* ── walking leg animation ── */
    int leg_off = 0;
    if (is_moving) leg_off = (tick % 2 == 0) ? 3 : -3;

    /* body */
    fill_circle(ren, cx, cy+br/2, br, orange);

    /* legs — animated when moving */
    if (facing == 0 || facing == 1) {
        /* moving left/right — legs bounce up/down */
        fill_rect(ren, cx-ts/5, cy+br+2,      ts/8, ts/6 + leg_off,  orange);
        fill_rect(ren, cx+ts/8, cy+br+2,      ts/8, ts/6 - leg_off,  orange);
    } else {
        /* moving up/down — legs side to side */
        fill_rect(ren, cx-ts/5+leg_off, cy+br+2, ts/8, ts/6, orange);
        fill_rect(ren, cx+ts/8-leg_off, cy+br+2, ts/8, ts/6, orange);
    }

    /* head — flip horizontally when facing left */
    int flip = (facing == 1) ? -1 : 1;
    fill_circle(ren, cx, cy-br/4, hr, orange);

    /* ears */
    fill_rect(ren, cx - flip*(hr+2), cy-hr,   hr/2+2, hr/2+2, orange);
    fill_rect(ren, cx - flip*hr,     cy-hr-2, hr/2-2, hr/2-2, pink);
    fill_rect(ren, cx + flip*(hr/2), cy-hr,   hr/2+2, hr/2+2, orange);
    fill_rect(ren, cx + flip*(hr/2+2), cy-hr-2, hr/2-2, hr/2-2, pink);

    /* eyes */
    int er = ts/14;
    int ex1 = cx - flip*(hr/2);
    int ex2 = cx + flip*(hr/2);
    int ey  = cy - hr/3;

    /* blink every 3 seconds */
    int blink = ((idle / 3000) % 10 == 0) ? 1 : 0;

    fill_circle(ren, ex1, ey, er+1, white);
    fill_circle(ren, ex2, ey, er+1, white);
    if (!blink) {
        fill_circle(ren, ex1, ey, er-1, green);
        fill_circle(ren, ex2, ey, er-1, green);
        fill_circle(ren, ex1, ey, er/2, black);
        fill_circle(ren, ex2, ey, er/2, black);
    } else {
        /* closed eyes — just a line */
        set_color(ren, dark);
        SDL_RenderDrawLine(ren, ex1-er, ey, ex1+er, ey);
        SDL_RenderDrawLine(ren, ex2-er, ey, ex2+er, ey);
    }

    /* nose */
    fill_rect(ren, cx - flip*2, cy-1, 5, 4, pink);

    /* whiskers */
    set_color(ren, dark);
    SDL_RenderDrawLine(ren, cx-2, cy,   cx - flip*(hr+4), cy-2);
    SDL_RenderDrawLine(ren, cx-2, cy+2, cx - flip*(hr+4), cy+4);
    SDL_RenderDrawLine(ren, cx+2, cy,   cx + flip*(hr+4), cy-2);
    SDL_RenderDrawLine(ren, cx+2, cy+2, cx + flip*(hr+4), cy+4);

    /* ── tail wag animation (idle) ── */
    int tail_wag = (int)((idle / 300) % 3) - 1;   /* -1, 0, 1 */
    int tx = cx + flip * hr;
    int ty = cy + br/2;
    set_color(ren, orange);
    SDL_RenderDrawLine(ren, tx,   ty, tx + flip*10, ty - 8 + tail_wag*4);
    SDL_RenderDrawLine(ren, tx+1, ty, tx + flip*11, ty - 8 + tail_wag*4);
    SDL_RenderDrawLine(ren, tx+2, ty, tx + flip*12, ty - 8 + tail_wag*4);

    /* stripes */
    set_color(ren, dark);
    SDL_RenderDrawLine(ren, cx-hr/2,   cy-hr,   cx-hr/2+2, cy-hr/2);
    SDL_RenderDrawLine(ren, cx,        cy-hr-2, cx,        cy-hr/2);
    SDL_RenderDrawLine(ren, cx+hr/2,   cy-hr,   cx+hr/2-2, cy-hr/2);
}

static void draw_hud(SDL_Renderer *ren, const GameState *gs)
{
    fill_rect(ren, 0, SCREEN_H-44, SCREEN_W, 44, (Color){15,15,15,255});

    /* ── Timer — show REMAINING time (60s limit) ── */
    Uint32 remaining_ms = 60000 > gs->elapsed_ms ? 60000 - gs->elapsed_ms : 0;
    Uint32 secs = remaining_ms / 1000;
    Uint32 mins = secs / 60;
    secs = secs % 60;
    /* turn red when under 10 seconds */
    Color timer_color = (remaining_ms <= 10000)
                        ? (Color){255, 60, 60, 255}
                        : (Color){255, 220, 0, 255};

    /* draw timer label box */
    fill_rect(ren, 10, SCREEN_H-38, 120, 30, (Color){40,40,40,255});

    /* draw MM:SS using pixel font */
    char time_str[8];
    time_str[0] = '0' + (mins / 10) % 10;
    time_str[1] = '0' + mins % 10;
    time_str[2] = ':';
    time_str[3] = '0' + secs / 10;
    time_str[4] = '0' + secs % 10;
    time_str[5] = '\0';

    static const unsigned char dg[10][7] = {
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

    int bx = 14;
    int by = SCREEN_H - 36;
    int bs = 2;

    for (int ci = 0; time_str[ci]; ci++) {
        char ch = time_str[ci];
        if (ch == ':') {
            /* draw colon as two dots */
            fill_rect(ren, bx+1, by+1,  bs, bs, timer_color);
            fill_rect(ren, bx+1, by+5,  bs, bs, timer_color);
            bx += 6;
            continue;
        }
        int d = ch - '0';
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 3; col++)
                if (dg[d][row] & (4 >> col))
                    fill_rect(ren, bx+col*(bs+1), by+row*(bs+1), bs, bs,
                              timer_color);
        bx += 4*(bs+1);
    }

    /* ── Move counter as actual number ── */
    fill_rect(ren, 145, SCREEN_H-38, 100, 30, (Color){40,40,40,255});

    int moves = gs->move_count;
    int hundreds = moves / 100;
    int tens2    = (moves % 100) / 10;
    int ones2    = moves % 10;

    bx = 150;
    by = SCREEN_H - 36;

    if (hundreds > 0) {
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 3; col++)
                if (dg[hundreds][row] & (4 >> col))
                    fill_rect(ren, bx+col*(bs+1), by+row*(bs+1), bs, bs,
                              (Color){60,140,220,255});
        bx += 4*(bs+1);
    }
    if (hundreds > 0 || tens2 > 0) {
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 3; col++)
                if (dg[tens2][row] & (4 >> col))
                    fill_rect(ren, bx+col*(bs+1), by+row*(bs+1), bs, bs,
                              (Color){60,140,220,255});
        bx += 4*(bs+1);
    }
    for (int row = 0; row < 7; row++)
        for (int col = 0; col < 3; col++)
            if (dg[ones2][row] & (4 >> col))
                fill_rect(ren, bx+col*(bs+1), by+row*(bs+1), bs, bs,
                          (Color){60,140,220,255});

    /* ── Level progress dots ── */
    for (int i = 0; i < gs->total_levels && i < 50; i++) {
        Color c = (i == gs->level_index) ? (Color){255,220,  0,255}
                : (i <  gs->level_index) ? (Color){ 80,160, 80,255}
                :                          (Color){ 50, 50, 50,255};
        fill_rect(ren, 260+i*13, SCREEN_H-30, 11, 18, c);
    }
}

/* ── Draw button helper ──────────────────────────────────────────────── */
static void draw_button(SDL_Renderer *ren, SDL_Rect r, Color bg, Color border,
                        Color text_color)
{
    fill_rect(ren, r.x, r.y, r.w, r.h, bg);
    set_color(ren, border);
    SDL_RenderDrawRect(ren, &r);
    /* inner border for depth */
    SDL_Rect inner = { r.x+2, r.y+2, r.w-4, r.h-4 };
    SDL_RenderDrawRect(ren, &inner);
    (void)text_color;
}

/* ── Draw pixel text (very simple A-Z 0-9) ───────────────────────────── */
static void draw_pixel_char(SDL_Renderer *ren, int x, int y, char c, int sz, Color clr)
{
    /* 5x7 pixel font for common letters */
    static const unsigned char font[][7] = {
        /* A */ {0x2,0x5,0x5,0x7,0x5,0x5,0x5},
        /* B */ {0x6,0x5,0x5,0x6,0x5,0x5,0x6},
        /* C */ {0x3,0x4,0x4,0x4,0x4,0x4,0x3},
        /* D */ {0x6,0x5,0x5,0x5,0x5,0x5,0x6},
        /* E */ {0x7,0x4,0x4,0x6,0x4,0x4,0x7},
        /* F */ {0x7,0x4,0x4,0x6,0x4,0x4,0x4},
        /* G */ {0x3,0x4,0x4,0x5,0x5,0x5,0x3},
        /* H */ {0x5,0x5,0x5,0x7,0x5,0x5,0x5},
        /* I */ {0x7,0x2,0x2,0x2,0x2,0x2,0x7},
        /* J */ {0x1,0x1,0x1,0x1,0x1,0x5,0x2},
        /* K */ {0x5,0x5,0x6,0x4,0x6,0x5,0x5},
        /* L */ {0x4,0x4,0x4,0x4,0x4,0x4,0x7},
        /* M */ {0x5,0x7,0x5,0x5,0x5,0x5,0x5},
        /* N */ {0x5,0x7,0x7,0x5,0x5,0x5,0x5},
        /* O */ {0x2,0x5,0x5,0x5,0x5,0x5,0x2},
        /* P */ {0x6,0x5,0x5,0x6,0x4,0x4,0x4},
        /* Q */ {0x2,0x5,0x5,0x5,0x5,0x3,0x1},
        /* R */ {0x6,0x5,0x5,0x6,0x5,0x5,0x5},
        /* S */ {0x3,0x4,0x4,0x2,0x1,0x1,0x6},
        /* T */ {0x7,0x2,0x2,0x2,0x2,0x2,0x2},
        /* U */ {0x5,0x5,0x5,0x5,0x5,0x5,0x2},
        /* V */ {0x5,0x5,0x5,0x5,0x5,0x2,0x2},
        /* W */ {0x5,0x5,0x5,0x5,0x7,0x7,0x5},
        /* X */ {0x5,0x5,0x2,0x2,0x2,0x5,0x5},
        /* Y */ {0x5,0x5,0x2,0x2,0x2,0x2,0x2},
        /* Z */ {0x7,0x1,0x1,0x2,0x4,0x4,0x7},
    };
    static const unsigned char num_font[][7] = {
        {0x7,0x5,0x5,0x5,0x5,0x5,0x7}, /* 0 */
        {0x2,0x6,0x2,0x2,0x2,0x2,0x7}, /* 1 */
        {0x7,0x1,0x1,0x7,0x4,0x4,0x7}, /* 2 */
        {0x7,0x1,0x1,0x7,0x1,0x1,0x7}, /* 3 */
        {0x5,0x5,0x5,0x7,0x1,0x1,0x1}, /* 4 */
        {0x7,0x4,0x4,0x7,0x1,0x1,0x7}, /* 5 */
        {0x7,0x4,0x4,0x7,0x5,0x5,0x7}, /* 6 */
        {0x7,0x1,0x1,0x1,0x1,0x1,0x1}, /* 7 */
        {0x7,0x5,0x5,0x7,0x5,0x5,0x7}, /* 8 */
        {0x7,0x5,0x5,0x7,0x1,0x1,0x7}, /* 9 */
    };

    const unsigned char *glyph = NULL;
    if (c >= 'A' && c <= 'Z') glyph = font[c - 'A'];
    else if (c >= '0' && c <= '9') glyph = num_font[c - '0'];
    if (!glyph) return;

    for (int row = 0; row < 7; row++)
        for (int col = 0; col < 3; col++)
            if (glyph[row] & (4 >> col))
                fill_rect(ren, x + col*sz, y + row*sz, sz, sz, clr);
}

static void draw_text(SDL_Renderer *ren, int x, int y, const char *str,
                      int sz, Color clr)
{
    for (int i = 0; str[i]; i++)
        draw_pixel_char(ren, x + i * (3*sz + sz), y, str[i], sz, clr);
}

/* ── Level complete screen with two buttons ─────────────────────────── */
static void draw_complete(SDL_Renderer *ren, int level)
{
    /* dark overlay */
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    fill_rect(ren, 0, 0, SCREEN_W, SCREEN_H, (Color){0, 0, 0, 160});
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    /* panel */
    int pw = 500, ph = 300;
    int px = SCREEN_W/2 - pw/2;
    int py = SCREEN_H/2 - ph/2;

    fill_rect(ren, px, py, pw, ph, (Color){20, 20, 20, 255});
    set_color(ren, (Color){255, 220, 0, 255});
    SDL_Rect panel = { px, py, pw, ph };
    SDL_RenderDrawRect(ren, &panel);
    SDL_Rect panel2 = { px+2, py+2, pw-4, ph-4 };
    SDL_RenderDrawRect(ren, &panel2);

    /* LEVEL CLEAR text */
    draw_text(ren, px + 60,  py + 40,  "LEVEL",  5, (Color){255,220,0,255});
    draw_text(ren, px + 80,  py + 90,  "CLEAR",  5, (Color){255,220,0,255});

    /* level number */
    char num[4];
    int n = level + 1;
    num[0] = (n >= 10) ? '0' + n/10 : ' ';
    num[1] = '0' + n%10;
    num[2] = '\0';
    draw_text(ren, px + 340, py + 60, num, 5, (Color){255,200,0,255});

    /* NEXT LEVEL button */
    g_btn_next = (SDL_Rect){ px + 40, py + 180, 180, 60 };
    draw_button(ren, g_btn_next,
                (Color){0, 150, 0, 255},
                (Color){0, 255, 0, 255},
                (Color){255,255,255,255});
    draw_text(ren, g_btn_next.x + 10, g_btn_next.y + 16, "NEXT", 3,
              (Color){255,255,255,255});

    /* QUIT button */
    g_btn_quit = (SDL_Rect){ px + 270, py + 180, 180, 60 };
    draw_button(ren, g_btn_quit,
                (Color){180, 0, 0, 255},
                (Color){255, 60, 60, 255},
                (Color){255,255,255,255});
    draw_text(ren, g_btn_quit.x + 30, g_btn_quit.y + 16, "QUIT", 3,
              (Color){255,255,255,255});
}


/* ── Menu button rects ───────────────────────────────────────────────── */
SDL_Rect g_btn_play = {0};
SDL_Rect g_btn_menu_quit = {0};

/* ── Draw main menu ──────────────────────────────────────────────────── */
void render_menu(SDL_Renderer *ren)
{
    /* background */
    fill_rect(ren, 0, 0, SCREEN_W, SCREEN_H, (Color){20, 20, 30, 255});

    /* decorative tiles as background pattern */
    for (int row = 0; row < SCREEN_H/64+1; row++)
        for (int col = 0; col < SCREEN_W/64+1; col++)
            fill_rect(ren, col*64, row*64, 62, 62, (Color){30, 30, 40, 255});

    /* title box */
    int tw = 500, th = 120;
    int tx = SCREEN_W/2 - tw/2;
    int ty = SCREEN_H/4 - th/2;
    fill_rect(ren, tx, ty, tw, th, (Color){15, 15, 25, 255});
    set_color(ren, (Color){255, 220, 0, 255});
    SDL_Rect tbox = { tx, ty, tw, th };
    SDL_RenderDrawRect(ren, &tbox);
    SDL_Rect tbox2 = { tx+3, ty+3, tw-6, th-6 };
    SDL_RenderDrawRect(ren, &tbox2);

    /* SOKOBAN title text */
    draw_text(ren, tx + 30,  ty + 20, "SOKOBAN", 8, (Color){255, 220, 0, 255});

    /* subtitle */
    draw_text(ren, SCREEN_W/2 - 160, ty + th + 20, "BOX PUSHING PUZZLE", 3,
              (Color){180, 180, 180, 255});

    /* SpongeBob decoration on the left */
    int sb_x = SCREEN_W/2 - 280;
    int sb_y = SCREEN_H/2 - 60;
    draw_spongebob(ren, sb_x, sb_y, 100, 0);

    /* cat decoration on the right */
    int cat_x = SCREEN_W/2 + 180;
    int cat_y = SCREEN_H/2 - 60;
    draw_cat(ren, cat_x, cat_y, 100);

    /* PLAY button */
    g_btn_play = (SDL_Rect){ SCREEN_W/2 - 120, SCREEN_H/2 + 60, 240, 70 };
    draw_button(ren, g_btn_play,
                (Color){0, 150, 0, 255},
                (Color){0, 255, 0, 255},
                (Color){255,255,255,255});
    draw_text(ren, g_btn_play.x + 50, g_btn_play.y + 18, "PLAY", 5,
              (Color){255, 255, 255, 255});

    /* QUIT button */
    g_btn_menu_quit = (SDL_Rect){ SCREEN_W/2 - 120, SCREEN_H/2 + 150, 240, 70 };
    draw_button(ren, g_btn_menu_quit,
                (Color){180, 0, 0, 255},
                (Color){255, 60, 60, 255},
                (Color){255,255,255,255});
    draw_text(ren, g_btn_menu_quit.x + 50, g_btn_menu_quit.y + 18, "QUIT", 5,
              (Color){255, 255, 255, 255});

    SDL_RenderPresent(ren);
}

void render_frame(SDL_Renderer *ren, const GameState *gs)
{
    g_gs_ptr = gs;   /* make game state available to draw_cat */
    set_color(ren, COLOR_BG);
    SDL_RenderClear(ren);

    int hud_h = 44;
    int ts    = get_tile_size(gs);

    int total_cols = SCREEN_W / ts + 1;
    int total_rows = (SCREEN_H - hud_h) / ts + 1;

    for (int row = 0; row < total_rows; row++)
        for (int col = 0; col < total_cols; col++)
            draw_floor(ren, col*ts, row*ts, ts);

    int off_x = (SCREEN_W - gs->cols * ts) / 2;
    int off_y = ((SCREEN_H - hud_h) - gs->rows * ts) / 2;

    for (int row = 0; row < gs->rows; row++) {
        for (int col = 0; col < gs->cols; col++) {
            int  px = off_x + col * ts;
            int  py = off_y + row * ts;
            Tile t  = gs->grid[row][col];
            if      (t == TILE_WALL)   draw_wall(ren, px, py, ts);
            else if (t == TILE_FLOOR)  draw_floor(ren, px, py, ts);
            else if (t == TILE_TARGET) draw_target(ren, px, py, ts);
        }
    }

    for (int b = 0; b < gs->box_count; b++) {
        int col = gs->boxes[b].x;
        int row = gs->boxes[b].y;
        int on  = (gs->grid[row][col] == TILE_TARGET);
        draw_spongebob(ren, off_x+col*ts, off_y+row*ts, ts, on);
    }

    draw_cat(ren, off_x+gs->player.x*ts, off_y+gs->player.y*ts, ts);
    draw_hud(ren, gs);

    if (gs->completed)
        draw_complete(ren, gs->level_index);

    SDL_RenderPresent(ren);
}

/* ── Pause button and screen ─────────────────────────────────────────── */
SDL_Rect g_btn_pause  = {0};
SDL_Rect g_btn_resume = {0};
SDL_Rect g_btn_pause_quit = {0};

void render_pause_button(SDL_Renderer *ren)
{
    g_btn_pause = (SDL_Rect){ SCREEN_W - 70, 10, 60, 30 };
    fill_rect(ren, g_btn_pause.x, g_btn_pause.y,
              g_btn_pause.w, g_btn_pause.h, (Color){60, 60, 60, 255});
    set_color(ren, (Color){200, 200, 200, 255});
    SDL_RenderDrawRect(ren, &g_btn_pause);

    /* two pause bars */
    fill_rect(ren, g_btn_pause.x + 16, g_btn_pause.y + 8,  8, 14, (Color){255,255,255,255});
    fill_rect(ren, g_btn_pause.x + 30, g_btn_pause.y + 8,  8, 14, (Color){255,255,255,255});
}

void render_pause_screen(SDL_Renderer *ren)
{
    /* dark overlay */
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    fill_rect(ren, 0, 0, SCREEN_W, SCREEN_H, (Color){0, 0, 0, 160});
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    /* panel */
    int pw = 400, ph = 280;
    int px = SCREEN_W/2 - pw/2;
    int py = SCREEN_H/2 - ph/2;

    fill_rect(ren, px, py, pw, ph, (Color){20, 20, 20, 255});
    set_color(ren, (Color){255, 220, 0, 255});
    SDL_Rect panel = { px, py, pw, ph };
    SDL_RenderDrawRect(ren, &panel);
    SDL_Rect panel2 = { px+2, py+2, pw-4, ph-4 };
    SDL_RenderDrawRect(ren, &panel2);

    /* PAUSED text */
    draw_text(ren, px + 80, py + 40, "PAUSED", 6, (Color){255, 220, 0, 255});

    /* RESUME button */
    g_btn_resume = (SDL_Rect){ px + 60, py + 130, 280, 60 };
    draw_button(ren, g_btn_resume,
                (Color){0, 150, 0, 255},
                (Color){0, 255, 0, 255},
                (Color){255,255,255,255});
    draw_text(ren, g_btn_resume.x + 40, g_btn_resume.y + 16,
              "RESUME", 4, (Color){255,255,255,255});

    /* QUIT button */
    g_btn_pause_quit = (SDL_Rect){ px + 60, py + 200, 280, 60 };
    draw_button(ren, g_btn_pause_quit,
                (Color){180, 0, 0, 255},
                (Color){255, 60, 60, 255},
                (Color){255,255,255,255});
    draw_text(ren, g_btn_pause_quit.x + 60, g_btn_pause_quit.y + 16,
              "QUIT", 4, (Color){255,255,255,255});

    SDL_RenderPresent(ren);
}