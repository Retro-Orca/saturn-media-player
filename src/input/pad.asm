; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

input_init:
    mov.l #SMPC+$79,r1
    mov #$60,r0
    mov.b r0,@r1
    mov.l #SMPC+$7D,r1
    mov #1,r0
    mov.b r0,@r1
    mov.l #SMPC+$7F,r1
    mov #0,r0
    mov.b r0,@r1
    rts
    nop
    ltorg

; デジタルパッドの選択線を切り替えて13個のボタンをまとめて読む
input_poll:
    sts.l pr,@-r15
    mov.l #SMPC+$75,r1
    mov #$60,r0
    mov.b r0,@r1
    bsr pad_settle
    nop
    mov.b @r1,r0
    and #8,r0
    shll8 r0
    shll2 r0
    shll2 r0
    mov r0,r3
    mov #$40,r0
    mov.b r0,@r1
    bsr pad_settle
    nop
    mov.b @r1,r0
    and #15,r0
    shll8 r0
    or r0,r3
    mov #$20,r0
    mov.b r0,@r1
    bsr pad_settle
    nop
    mov.b @r1,r0
    and #15,r0
    shll2 r0
    shll2 r0
    or r0,r3
    mov #0,r0
    mov.b r0,@r1
    bsr pad_settle
    nop
    mov.b @r1,r0
    and #15,r0
    or r0,r3
    mov #$60,r0
    mov.b r0,@r1
    mov.l #$8FFF,r0
    xor r0,r3
    ifdef TEST_BUILD
    mov.l #AUTO_TEST+$20000000,r1
    mov.l @r1,r0
    or r0,r3
    mov #0,r0
    mov.l r0,@r1
    endif
    mov.l @(PAD_HELD,r14),r0
    not r0,r0
    and r3,r0
    mov.l r0,@(PAD_EDGE,r14)
    mov.l r3,@(PAD_HELD,r14)
    lds.l @r15+,pr
    rts
    nop
pad_settle:
    mov #32,r2
pad_settle_loop:
    dt r2
    bf pad_settle_loop
    rts
    nop
    ltorg
