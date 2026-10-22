; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; 4x4の画素を8x8セルの右下、左下、右上、左上の順で組み立てる
decode_quadrant macro adjustment
    mov r7,r0
    mov.l #511,r1
    and r1,r0
    shll2 r0
    shll2 r0
    shll r0
    mov r11,r2
    add r0,r2
    mov.l @r2+,r0
    mov.l @r2+,r1
    mov.l r0,@r10
    mov.l r1,@(4,r10)
    mov.l @r2+,r0
    mov.l @r2+,r1
    mov.l r0,@(16,r10)
    mov.l r1,@(20,r10)
    mov.l @r2+,r0
    mov.l @r2+,r1
    mov.l r0,@(32,r10)
    mov.l r1,@(36,r10)
    mov.l @r2+,r0
    mov.l @r2+,r1
    mov.l r0,@(48,r10)
    mov.l r1,@(52,r10)
    add #64,r10
    shlr8 r7
    shlr r7
    add #adjustment,r10
    endm

; 圧縮番号を高速ワークRAMへ復元し、画素の転送はSCU DMAへ任せる
video_decode:
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l r10,@-r15
    mov.l r11,@-r15
    mov.l r12,@-r15
    mov.l r13,@-r15
    mov r4,r8
    mov r4,r9
    mov.l #140,r0
    add r0,r9
    mov.l #FRAME_RAM,r10
    mov.l @(DICT_BANK,r14),r0
    mov.l #12288,r1
    mulu.w r1,r0
    sts macl,r11
    mov.l #CODEBOOK_RAM,r0
    add r0,r11
    mov.l #FRAME_CHANGED,r1
    mov #0,r0
    mov.l r0,@r1
    mov.l #140,r12
decode_bank_byte:
    mov.b @r8+,r13
    shll16 r13
    shll8 r13
    mov #8,r6
decode_cell:
    shll r13
    bt decode_changed
    mov.l #128,r0
    add r0,r10
    bra decode_advance
    nop
decode_changed:
    mov.l #FRAME_CHANGED,r1
    mov #1,r0
    mov.l r0,@r1
    mov.b @r9+,r3
    extu.b r3,r3
    shll2 r3
    shll2 r3
    shll r3
    mov.b @r9+,r7
    extu.b r7,r7
    shll8 r7
    mov.b @r9+,r0
    extu.b r0,r0
    or r0,r7
    shll8 r7
    mov.b @r9+,r0
    extu.b r0,r0
    or r0,r7
    shll8 r7
    mov.b @r9+,r0
    extu.b r0,r0
    or r0,r7
    add #72,r10
    decode_quadrant -72
    decode_quadrant -120
    decode_quadrant -72
    or r3,r7
    decode_quadrant 64
decode_advance:
    dt r6
    bt decode_cells_done
    bra decode_cell
    nop
decode_cells_done:
    dt r12
    bt decode_done
    bra decode_bank_byte
    nop
decode_done:
    mov.l @r15+,r13
    mov.l @r15+,r12
    mov.l @r15+,r11
    mov.l @r15+,r10
    mov.l @r15+,r9
    mov.l @r15+,r8
    rts
    nop
    ltorg
