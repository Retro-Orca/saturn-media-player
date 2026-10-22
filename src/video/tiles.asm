; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; r4の辞書を非表示側へ転送する 表示中の辞書は変更しない
video_dictionary:
    sts.l pr,@-r15
    mov.l @(DICT_BANK,r14),r0
    mov r0,r5
    shll16 r5
    shll r5
    mov.l #VRAM,r0
    add r0,r5
    mov.l #32768,r6
    call copy_words
    lds.l @r15+,pr
    rts
    nop
    ltorg

; r4の1120個の番号をVDP2の40列28行のパターン表へ展開する
video_frame:
    mov.l @(MAP_BANK,r14),r0
    shll8 r0
    shll2 r0
    shll2 r0
    shll2 r0
    mov.l #VRAM+$40000,r5
    add r0,r5
    mov.l @(DICT_BANK,r14),r3
    shll8 r3
    shll2 r3
    shll2 r3
    mov #28,r6
video_frame_row:
    mov #40,r7
video_frame_column:
    mov.b @r4+,r0
    extu.b r0,r0
    shll2 r0
    add r3,r0
    mov #0,r1
    mov.w r1,@r5
    add #2,r5
    mov.w r0,@r5
    add #2,r5
    dt r7
    bf video_frame_column
    add #96,r5
    dt r6
    bf video_frame_row
    rts
    nop
    ltorg

video_present:
    ; 辞書を読むバンクとCPUが次の辞書を書くバンクを交互に入れ替える
    mov.l #VDP2+$12,r1
    mov.l #$EEEE,r0
    mov.w r0,@r1
    add #4,r1
    mov.w r0,@r1
    mov.l @(DICT_BANK,r14),r0
    shll2 r0
    mov.l #VDP2+$12,r1
    add r0,r1
    mov.l #$4444,r0
    mov.w r0,@r1
    mov.l @(MAP_BANK,r14),r0
    add #16,r0
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
