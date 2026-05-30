#include "sound.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>
#include <math.h>

#define SAMPLE_RATE  44100
#define CHUNK_SIZE   1024

/* ── Stored chunks ───────────────────────────────────────────────────── */
static Mix_Chunk *snd_push     = NULL;
static Mix_Chunk *snd_target   = NULL;
static Mix_Chunk *snd_complete = NULL;
static Mix_Chunk *snd_warning  = NULL;

/* ── Generate a simple sine wave chunk ──────────────────────────────── */
static Mix_Chunk *make_tone(int freq, int duration_ms, float volume)
{
    int samples  = SAMPLE_RATE * duration_ms / 1000;
    int buf_size = samples * 2;   /* 16-bit mono */

    Uint8 *buf = (Uint8 *)SDL_malloc(buf_size);
    if (!buf) return NULL;

    Sint16 *s = (Sint16 *)buf;
    for (int i = 0; i < samples; i++) {
        float t    = (float)i / SAMPLE_RATE;
        float wave = (float)sin(2.0 * 3.14159265 * freq * t);
        /* fade out to avoid click */
        float env  = (i < 100) ? (float)i / 100.0f
                   : (i > samples - 100) ? (float)(samples - i) / 100.0f
                   : 1.0f;
        s[i] = (Sint16)(wave * env * volume * 32767.0f);
    }

    Mix_Chunk *chunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
    if (!chunk) { SDL_free(buf); return NULL; }

    chunk->allocated = 1;
    chunk->abuf      = buf;
    chunk->alen      = buf_size;
    chunk->volume    = MIX_MAX_VOLUME;
    return chunk;
}

/* ── Generate a two-tone beep ────────────────────────────────────────── */
static Mix_Chunk *make_two_tone(int freq1, int freq2, int dur_ms, float vol)
{
    int samples  = SAMPLE_RATE * dur_ms / 1000;
    int half     = samples / 2;
    int buf_size = samples * 2;

    Uint8 *buf = (Uint8 *)SDL_malloc(buf_size);
    if (!buf) return NULL;

    Sint16 *s = (Sint16 *)buf;
    for (int i = 0; i < samples; i++) {
        float t    = (float)i / SAMPLE_RATE;
        int   freq = (i < half) ? freq1 : freq2;
        float wave = (float)sin(2.0 * 3.14159265 * freq * t);
        float env  = (i < 100) ? (float)i / 100.0f
                   : (i > samples - 100) ? (float)(samples - i) / 100.0f
                   : 1.0f;
        s[i] = (Sint16)(wave * env * vol * 32767.0f);
    }

    Mix_Chunk *chunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
    if (!chunk) { SDL_free(buf); return NULL; }

    chunk->allocated = 1;
    chunk->abuf      = buf;
    chunk->alen      = buf_size;
    chunk->volume    = MIX_MAX_VOLUME;
    return chunk;
}

/* ── Generate a three-tone victory sound ─────────────────────────────── */
static Mix_Chunk *make_victory(void)
{
    int dur_ms   = 600;
    int samples  = SAMPLE_RATE * dur_ms / 1000;
    int third    = samples / 3;
    int buf_size = samples * 2;

    Uint8 *buf = (Uint8 *)SDL_malloc(buf_size);
    if (!buf) return NULL;

    Sint16 *s  = (Sint16 *)buf;
    int freqs[3] = {523, 659, 784};   /* C5, E5, G5 — major chord */

    for (int i = 0; i < samples; i++) {
        float t    = (float)i / SAMPLE_RATE;
        int   freq = freqs[i / third < 3 ? i / third : 2];
        float wave = (float)sin(2.0 * 3.14159265 * freq * t);
        float env  = (i < 100) ? (float)i / 100.0f
                   : (i > samples - 200) ? (float)(samples - i) / 200.0f
                   : 1.0f;
        s[i] = (Sint16)(wave * env * 0.6f * 32767.0f);
    }

    Mix_Chunk *chunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
    if (!chunk) { SDL_free(buf); return NULL; }

    chunk->allocated = 1;
    chunk->abuf      = buf;
    chunk->alen      = buf_size;
    chunk->volume    = MIX_MAX_VOLUME;
    return chunk;
}

/* ── Init ────────────────────────────────────────────────────────────── */
int sound_init(void)
{
    if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, 1, CHUNK_SIZE) < 0)
        return 0;   /* fail silently — game still works without sound */

    snd_push     = make_tone(300,     80,  0.4f);   /* low short thud   */
    snd_target   = make_two_tone(440, 660, 200, 0.5f); /* ding!          */
    snd_complete = make_victory();                       /* C-E-G fanfare */
    snd_warning  = make_tone(880,     150, 0.5f);   /* high beep        */

    return 1;
}

/* ── Quit ────────────────────────────────────────────────────────────── */
void sound_quit(void)
{
    if (snd_push)     Mix_FreeChunk(snd_push);
    if (snd_target)   Mix_FreeChunk(snd_target);
    if (snd_complete) Mix_FreeChunk(snd_complete);
    if (snd_warning)  Mix_FreeChunk(snd_warning);
    Mix_CloseAudio();
}

/* ── Play functions ──────────────────────────────────────────────────── */
void sound_play_push(void)    { if (snd_push)     Mix_PlayChannel(-1, snd_push,     0); }
void sound_play_target(void)  { if (snd_target)   Mix_PlayChannel(-1, snd_target,   0); }
void sound_play_complete(void){ if (snd_complete) Mix_PlayChannel(-1, snd_complete, 0); }
void sound_play_warning(void) { if (snd_warning)  Mix_PlayChannel(-1, snd_warning,  0); }