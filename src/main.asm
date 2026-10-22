; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

    cpu SH7600
    supmode on
    padding on
    include "platform/hardware.inc"
    org $06004000

entry:
    mov.l #STACK,r15
    mov #$F0,r0
    extu.b r0,r0
    ldc r0,sr
    mov.l #$25FE00A0,r1
    mov #-1,r0
    mov.l r0,@r1
    mov.l #STATE,r14
    mov r14,r1
    mov #64,r2
    mov #0,r0
entry_clear:
    mov.l r0,@r1
    add #4,r1
    dt r2
    bf entry_clear
    mov.l #STATE+96,r1
    mov.l #$53565431,r0
    mov.l r0,@r1
    mov.l #$53544154,r0
    mov.l r0,@(4,r1)
    call video_init
    call input_init
    call audio_init
    call cd_init
    tst r0,r0
    bt entry_cd_fail
    call video_enable_counter
    mov #1,r0
    mov.l r0,@(DIRTY_MENU,r14)
    bra main_loop
    nop
entry_cd_fail:
    mov #3,r0
    mov.l r0,@(MODE,r14)
main_loop:
    call stream_pump
    call input_poll
    call player_controls
    mov.l @(MODE,r14),r0
    cmp/eq #1,r0
    bt main_play
    call video_wait_vblank
    mov.l @(MODE,r14),r0
    cmp/eq #2,r0
    bt main_loop
    cmp/eq #4,r0
    bt main_buffer
    cmp/eq #3,r0
    bt main_error
    mov.l @(DIRTY_MENU,r14),r0
    tst r0,r0
    bt main_loop
    call menu_draw
    bra main_loop
    nop
main_play:
    call player_tick
    bra main_loop
    nop
main_buffer:
    call player_buffering_tick
    bra main_loop
    nop
main_error:
    call error_draw
    bra main_loop
    nop
    ltorg

    include "platform/memory.asm"
    include "input/pad.asm"
    include "video/setup.asm"
    include "video/tiles.asm"
    include "video/vblank.asm"
    include "audio/setup.asm"
    include "audio/pcm.asm"
    include "cd/command.asm"
    include "cd/setup.asm"
    include "cd/read.asm"
    include "player/stream.asm"
    include "player/playback.asm"
    include "player/controls.asm"
    include "ui/bitmap.asm"
    include "ui/menu.asm"
    include "build/catalog.inc"
    align 4
font_data:
    binclude "build/font.bin"
    end entry
