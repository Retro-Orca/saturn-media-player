; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

audio_stop:
    mov.l #SCSP,r1
    mov.l #$1000,r0
    mov.w r0,@r1
    rts
    nop
    ltorg

; r4は現在グループの先頭 r5は再開するフレーム番号
audio_start:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l #AUDIO_ERROR,r1
    mov.l @r1,r0
    tst r0,r0
    bf audio_start_done
    mov r5,r8
    mov.l @(PLAY_GROUP,r14),r0
    and #1,r0
    shll8 r0
    shll2 r0
    shll2 r0
    shll2 r0
    mov.l #$2000,r9
    add r0,r9
    mov.l #SOUND_OFFSET,r0
    add r0,r4
    mov.l #SOUND_RAM,r5
    add r9,r5
    mov.l #SOUND_SIZE,r6
    call copy_words
    call audio_stop
    ; フレーム位置から4410サンプル中の再開点を求める
    mov.l #735,r0
    mulu.w r8,r0
    sts macl,r2
    shlr r2
    mov r2,r0
    shll r0
    add r9,r0
    mov.l #SCSP,r1
    mov.w r0,@(2,r1)
    mov #0,r0
    mov.w r0,@(4,r1)
    mov.l #4410,r0
    sub r2,r0
    mov.w r0,@(6,r1)
    mov.l #$1800,r0
    mov.w r0,@r1
audio_start_done:
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
