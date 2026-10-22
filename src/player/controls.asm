; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

player_controls:
    sts.l pr,@-r15
    mov.l @(MODE,r14),r0
    tst r0,r0
    bt controls_menu
    mov.l @(PAD_EDGE,r14),r0
    tst #BTN_X,r0
    bt controls_playback
    call player_to_menu
    bra controls_done
    nop
controls_playback:
    mov.l @(MODE,r14),r0
    cmp/eq #1,r0
    bt controls_pause_check
    cmp/eq #2,r0
    bf controls_done
controls_pause_check:
    mov.l @(PAD_EDGE,r14),r0
    tst #BTN_Y,r0
    bt controls_toggle_check
    mov #2,r0
    mov.l r0,@(MODE,r14)
    call audio_stop
    bra controls_seek_check
    nop
controls_toggle_check:
    tst #BTN_Z,r0
    bt controls_seek_check
    mov.l @(MODE,r14),r0
    cmp/eq #2,r0
    bt controls_resume
    mov #2,r0
    mov.l r0,@(MODE,r14)
    call audio_stop
    bra controls_seek_check
    nop
controls_resume:
    call audio_start
    call video_wait_vblank
    mov #1,r0
    mov.l r0,@(MODE,r14)
controls_seek_check:
    mov.l @(PAD_EDGE,r14),r0
    mov.l #BTN_L,r1
    tst r1,r0
    bf controls_seek_short
    tst #BTN_R,r0
    bt controls_done
    mov.l #150,r4
    bra controls_seek
    nop
controls_seek_short:
    mov #25,r4
controls_seek:
    mov.l @(PLAY_GROUP,r14),r0
    add r0,r4
    mov.l @(MOVIE_GROUPS,r14),r1
    cmp/hs r1,r4
    bf controls_seek_valid
    mov r1,r4
    add #-1,r4
controls_seek_valid:
    call player_seek
    bra controls_done
    nop
controls_menu:
    mov.l @(PAD_EDGE,r14),r0
    tst #BTN_DOWN,r0
    bt controls_menu_up
    mov.l @(SELECTION,r14),r0
    add #1,r0
    mov.l #MOVIE_COUNT,r1
    cmp/hs r1,r0
    bf controls_store_selection
    mov #0,r0
    bra controls_store_selection
    nop
controls_menu_up:
    tst #BTN_UP,r0
    bt controls_menu_play
    mov.l @(SELECTION,r14),r0
    add #-1,r0
    cmp/pz r0
    bt controls_store_selection
    mov.l #MOVIE_COUNT-1,r0
controls_store_selection:
    mov.l r0,@(SELECTION,r14)
    mov #1,r0
    mov.l r0,@(DIRTY_MENU,r14)
controls_menu_play:
    mov.l @(PAD_EDGE,r14),r0
    mov.l #BTN_A+BTN_START,r1
    tst r1,r0
    bt controls_done
    call player_open
controls_done:
    lds.l @r15+,pr
    rts
    nop
    ltorg
