; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; フリーランニングタイマーで垂直同期間の経過クロックを記録する
read_timer macro
    mov.l #$FFFFFE12,r1
    mov.b @r1,r0
    extu.b r0,r0
    shll8 r0
    mov r0,r2
    add #1,r1
    mov.b @r1,r0
    extu.b r0,r0
    or r2,r0
    endm

video_enable_counter:
    mov.l #$FFFFFE10,r1
    mov #1,r0
    mov.b r0,@r1
    mov.l #$FFFFFE16,r1
    mov #2,r0
    mov.b r0,@r1
    read_timer
    mov.l #STATE+108,r1
    mov.l r0,@r1
    rts
    nop
    ltorg

video_tick_time:
    read_timer
    mov.l #STATE+108,r1
    mov.l @r1,r2
    mov.l r0,@r1
    sub r2,r0
    extu.w r0,r0
    mov.l #STATE+112,r1
    mov.l r0,@r1
    mov.l #STATE+104,r1
    mov.l @r1,r2
    add r0,r2
    mov.l r2,@r1
    mov.l @(MODE,r14),r3
    mov #1,r2
    cmp/eq r2,r3
    bf video_tick_done
    mov.l #STATE+116,r1
    mov.l @r1,r2
    cmp/hi r2,r0
    bf video_tick_limit
    mov.l r0,@r1
video_tick_limit:
    mov.l #5200,r2
    cmp/hi r2,r0
    bf video_tick_done
    mov.l #STATE+120,r1
    mov.l @r1,r0
    add #1,r0
    mov.l r0,@r1
video_tick_done:
    rts
    nop
    ltorg
