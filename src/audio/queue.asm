; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; r4のグループを再生中の位置より先にある循環バッファへ補充する
audio_queue:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l r10,@-r15
    mov r4,r8
    call group_address
    mov r0,r9
    mov.l #SOUND_OFFSET,r0
    add r0,r9
    mov.l #AUDIO_ORIGIN,r1
    mov.l @r1,r0
    sub r0,r8
    mov r8,r0
    and #7,r0
    mov.l #SOUND_SIZE,r1
    mulu.w r1,r0
    sts macl,r8
    mov.l #AUDIO_SKIP,r1
    mov.l @r1,r0
    sub r0,r8
    cmp/pz r8
    bt audio_queue_position
    mov.l #AUDIO_RING_SIZE,r0
    add r0,r8
audio_queue_position:
    mov.l #AUDIO_RING_SIZE,r10
    sub r8,r10
    mov.l #SOUND_SIZE,r0
    cmp/hi r0,r10
    bf audio_queue_first
    mov r0,r10
audio_queue_first:
    mov r9,r4
    mov.l #SOUND_RAM+AUDIO_RING,r5
    add r8,r5
    mov r10,r6
    call copy_words
    mov.l #SOUND_SIZE,r6
    sub r10,r6
    tst r6,r6
    bt audio_queue_guard
    mov r9,r4
    add r10,r4
    mov.l #SOUND_RAM+AUDIO_RING,r5
    call copy_words
audio_queue_guard:
    ; ループ終端の補間用サンプルも先頭に合わせる
    mov.l #SOUND_RAM+AUDIO_RING,r1
    mov.w @r1,r0
    mov.l #SOUND_RAM+AUDIO_RING+AUDIO_RING_SIZE,r1
    mov.w r0,@r1
    mov.l #AUDIO_QUEUED,r1
    mov.l @r1,r0
    add #1,r0
    mov.l r0,@r1
    mov.l @r15+,r10
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
