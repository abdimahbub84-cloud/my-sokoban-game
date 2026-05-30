#ifndef SOUND_H
#define SOUND_H

/* ── Initialize and shutdown sound system ────────────────────────────── */
int  sound_init(void);
void sound_quit(void);

/* ── Play sound effects ──────────────────────────────────────────────── */
void sound_play_push(void);       /* box pushed          */
void sound_play_target(void);     /* box landed on spot  */
void sound_play_complete(void);   /* level complete      */
void sound_play_warning(void);    /* time running out    */

#endif