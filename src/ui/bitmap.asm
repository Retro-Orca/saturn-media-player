; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

bitmap_clear:
    mov.l #VRAM,r1
    mov.l #131072,r2
    mov.l #$8000,r0
bitmap_clear_loop:
    mov.w r0,@r1
    add #2,r1
    dt r2
    bf bitmap_clear_loop
    rts
    nop
    ltorg

; r4は1ビット画像 r5とr6は座標 r7は一行のバイト数 r3は文字色 r2は背景色
bitmap_mask:
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l r10,@-r15
    mov.l r11,@-r15
    mov.l r12,@-r15
    mov.l r13,@-r15
    mov r7,r8
    mov r4,r12
    mov r3,r10
    mov r2,r11
    mov r6,r9
    shll8 r9
    shll2 r9
    shll r5
    add r5,r9
    mov.l #VRAM,r0
    add r0,r9
    mov #16,r13
bitmap_mask_row:
    mov r8,r7
bitmap_mask_byte:
    mov.b @r12+,r2
    mov #8,r6
bitmap_mask_bit:
    mov r2,r0
    tst #$80,r0
    bt bitmap_mask_background
    mov r10,r0
    bra bitmap_mask_pixel
    nop
bitmap_mask_background:
    mov r11,r0
bitmap_mask_pixel:
    mov.w r0,@r9
    add #2,r9
    shll r2
    dt r6
    bf bitmap_mask_bit
    dt r7
    bf bitmap_mask_byte
    mov r8,r0
    shll2 r0
    shll2 r0
    mov.l #1024,r1
    sub r0,r1
    add r1,r9
    dt r13
    bf bitmap_mask_row
    mov.l @r15+,r13
    mov.l @r15+,r12
    mov.l @r15+,r11
    mov.l @r15+,r10
    mov.l @r15+,r9
    mov.l @r15+,r8
    rts
    nop
    ltorg

; ASCII文字列を8ドット間隔で描く 日本語の動画名は別に用意した画像を使う
ui_text:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l r10,@-r15
    mov.l r11,@-r15
    mov.l r12,@-r15
    mov r4,r8
    mov r5,r9
    mov r6,r10
    mov r3,r11
    mov r2,r12
ui_text_loop:
    mov.b @r8+,r0
    extu.b r0,r0
    tst r0,r0
    bt ui_text_done
    add #-32,r0
    shll2 r0
    shll2 r0
    mov.l #font_data,r4
    add r0,r4
    mov r9,r5
    mov r10,r6
    mov r11,r3
    mov r12,r2
    mov #1,r7
    call bitmap_mask
    add #8,r9
    bra ui_text_loop
    nop
ui_text_done:
    mov.l @r15+,r12
    mov.l @r15+,r11
    mov.l @r15+,r10
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg

ui_show:
    mov.l #VDP2+$20,r1
    mov.l #$0101,r0
    mov.w r0,@r1
    rts
    nop
    ltorg
