; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

SMPC             equ $20100000
VDP2             equ $25F80000
VRAM             equ $25E00000
SCSP             equ $25B00000
SOUND_RAM        equ $25A00000
CD               equ $25890000
CD_DATA          equ $25898000
STATE            equ $060D0000
STACK            equ $060FF000
STREAM_RAM       equ $20200000
GROUP_SIZE       equ 55296
GROUP_SECTORS    equ 27
FRAME_SIZE       equ 1120
SOUND_OFFSET     equ 46208
SOUND_SIZE       equ 8820

PAD_HELD         equ 0
PAD_EDGE         equ 4
SELECTION        equ 8
MODE             equ 12
MOVIE            equ 16
PLAY_GROUP       equ 20
PLAY_FRAME       equ 24
FRAME_NUMBER     equ 28
FILL_GROUP       equ 32
FILL_SECTOR      equ 36
MOVIE_GROUPS     equ 40
MOVIE_FRAMES     equ 44
DICT_BANK        equ 48
MAP_BANK         equ 52
DIRTY_MENU       equ 56
VBLANK_COUNT     equ 60
CD_ERROR         equ STATE+64
CD_SECTORS_LEFT  equ STATE+68
PREPARED_GROUP   equ STATE+72
AUDIO_ERROR      equ STATE+76
UNDERRUNS        equ STATE+80
AUTO_TEST        equ STATE+84
TIMER_TICKS      equ STATE+104

BTN_UP           equ $0010
BTN_DOWN         equ $0020
BTN_A            equ $0400
BTN_START        equ $0800
BTN_X            equ $0004
BTN_Y            equ $0002
BTN_Z            equ $0001
BTN_L            equ $8000
BTN_R            equ $0008

; r0からr7は呼び出し先が使用できる r8からr14は保存しr14を状態領域に固定する
call macro target
    mov.l #target,r0
    jsr @r0
    nop
    endm
