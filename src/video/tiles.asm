; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

video_dictionary:
    sts.l pr,@-r15
    profile_start 220
    mov.l @(DICT_BANK,r14),r0
    mov.l #12288,r1
    mulu.w r1,r0
    sts macl,r5
    mov.l #CODEBOOK_RAM,r0
    add r0,r5
    mov.l #12288,r6
    call copy_words
    profile_end 220,192
    lds.l @r15+,pr
    rts
    nop
    ltorg

video_frame:
    sts.l pr,@-r15
    profile_start 200
    call video_decode
    profile_end 200,172
    mov.l #FRAME_CHANGED,r1
    mov.l @r1,r0
    tst r0,r0
    bt video_frame_done
    profile_start 204
    call video_upload
    profile_end 204,176
video_frame_done:
    lds.l @r15+,pr
    rts
    nop
    ltorg

; r4のマップに画面の各8x8セル固有の画素領域を割り当てる
video_map_init:
    mov #28,r6
video_map_row:
    mov #40,r7
video_map_cell:
    mov #0,r0
    mov.w r0,@r4
    add #2,r4
    mov.w r5,@r4
    add #2,r4
    add #4,r5
    dt r7
    bf video_map_cell
    add #96,r4
    dt r6
    bf video_map_row
    rts
    nop

video_present:
    mov.l #FRAME_CHANGED,r1
    mov.l @r1,r0
    tst r0,r0
    bf video_present_changed
    rts
    nop
video_present_changed:
    mov.l #VDP2+$10,r1
    mov #8,r2
    mov.l #$EEEE,r0
video_present_cpu:
    mov.w r0,@r1
    add #2,r1
    dt r2
    bf video_present_cpu
    mov.l @(MAP_BANK,r14),r0
    shll2 r0
    shll r0
    mov.l #VDP2+$10,r1
    add r0,r1
    rept 2
    mov.l #$00EE,r0
    mov.w r0,@r1
    add #2,r1
    mov.l #$4444,r0
    mov.w r0,@r1
    add #2,r1
    endm
    mov.l @(MAP_BANK,r14),r0
    shll2 r0
    shll2 r0
    add #12,r0
    mov r0,r2
    shll8 r0
    or r2,r0
    mov.l #VDP2+$40,r1
    mov.w r0,@r1
    add #2,r1
    mov.w r0,@r1
    mov.l #VDP2+$20,r1
    mov.l #$0101,r0
    mov.w r0,@r1
    mov.l @(MAP_BANK,r14),r0
    xor #1,r0
    mov.l r0,@(MAP_BANK,r14)
    rts
    nop
    ltorg
