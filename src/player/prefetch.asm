; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; 次の辞書を2KiBずつ先に転送し、0.2秒ごとの処理集中を避ける
player_prefetch:
    sts.l pr,@-r15
    mov.l r8,@-r15
    profile_start 208
    mov.l @(PLAY_GROUP,r14),r4
    add #1,r4
    mov.l @(FILL_GROUP,r14),r0
    cmp/hs r0,r4
    bt player_prefetch_done
    mov.l #NEXT_PREPARED,r1
    mov.l @r1,r0
    cmp/eq r4,r0
    bt player_prefetch_done
    mov.l #NEXT_COPY_GROUP,r1
    mov.l @r1,r0
    cmp/eq r4,r0
    bt player_prefetch_progress
    mov.l r4,@r1
    mov.l #NEXT_COPY_BYTES,r1
    mov #0,r0
    mov.l r0,@r1
player_prefetch_progress:
    call group_address
    mov r0,r4
    mov.l #NEXT_COPY_BYTES,r1
    mov.l @r1,r8
    add r8,r4
    mov.l @(PLAY_GROUP,r14),r0
    xor #1,r0
    and #1,r0
    mov.l #12288,r1
    mulu.w r1,r0
    sts macl,r5
    mov.l #CODEBOOK_RAM,r0
    add r0,r5
    add r8,r5
    mov.l #2048,r6
    call copy_words
    mov.l #2048,r0
    add r0,r8
    mov.l #NEXT_COPY_BYTES,r1
    mov.l r8,@r1
    mov.l #12288,r0
    cmp/eq r0,r8
    bf player_prefetch_done
    mov.l #NEXT_COPY_GROUP,r1
    mov.l @r1,r0
    mov.l #NEXT_PREPARED,r1
    mov.l r0,@r1
player_prefetch_done:
    profile_end 208,180
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
