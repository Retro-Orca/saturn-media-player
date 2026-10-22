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
STREAM_RAM       equ $26020000
GROUP_SIZE       equ 55296
GROUP_SECTORS    equ 27
FRAME_DIRECTORY  equ 12288
SOUND_OFFSET     equ 46476
SOUND_SIZE       equ 8820
CODEBOOK_RAM     equ $06090000
FRAME_RAM        equ $06098000
AUDIO_RING       equ $10000
AUDIO_RING_SIZE  equ 70560
AUDIO_ORIGIN     equ STATE+128
AUDIO_SKIP       equ STATE+132
AUDIO_ACTIVE     equ STATE+136
AUDIO_STARTS     equ STATE+140
AUDIO_QUEUED     equ STATE+144
FRAME_CHANGED    equ STATE+148
NEXT_PREPARED    equ STATE+152
NEXT_COPY_BYTES  equ STATE+156
NEXT_COPY_GROUP  equ STATE+160
AUDIO_NEXT       equ STATE+164
AUDIO_COPY_BYTES equ STATE+168

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

profile_start macro slot
    ifdef TEST_BUILD
    mov.l #$FFFFFE12,r1
    mov.b @r1,r0
    extu.b r0,r0
    shll8 r0
    mov r0,r2
    add #1,r1
    mov.b @r1,r0
    extu.b r0,r0
    or r2,r0
    mov.l #STATE+slot,r1
    mov.l r0,@r1
    endif
    endm

profile_end macro startslot,resultslot
    ifdef TEST_BUILD
    mov.l #$FFFFFE12,r1
    mov.b @r1,r0
    extu.b r0,r0
    shll8 r0
    mov r0,r2
    add #1,r1
    mov.b @r1,r0
    extu.b r0,r0
    or r2,r0
    mov.l #STATE+startslot,r1
    mov.l @r1,r2
    sub r2,r0
    extu.w r0,r0
    mov.l #STATE+resultslot,r1
    mov.l @r1,r2
    cmp/hi r2,r0
    movt r3
    neg r3,r3
    xor r2,r0
    and r3,r0
    xor r2,r0
    mov.l r0,@r1
    endif
    endm
