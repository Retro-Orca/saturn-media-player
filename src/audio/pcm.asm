; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

audio_stop:
    mov.l #SCSP,r1
    mov.l #$1000,r0
    mov.w r0,@r1
    mov.l #AUDIO_ACTIVE,r1
    mov #0,r0
    mov.l r0,@r1
    rts
    nop
    ltorg

; 開始と一時停止からの再開だけKEY_ONし、区間の境界では止めない
audio_start:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l #AUDIO_ERROR,r1
    mov.l @r1,r0
    tst r0,r0
    bf audio_start_done
    call audio_stop
    mov.l @(PLAY_GROUP,r14),r8
    mov.l #AUDIO_ORIGIN,r1
    mov.l r8,@r1
    mov.l @(PLAY_FRAME,r14),r0
    mov.l #735,r1
    mulu.w r1,r0
    sts macl,r0
    ; 735バイト毎フレームは常に16ビットのサンプル境界へ切り下げる
    shlr r0
    shll r0
    mov.l #AUDIO_SKIP,r1
    mov.l r0,@r1
    mov.l @(FILL_GROUP,r14),r9
audio_start_fill:
    cmp/hs r9,r8
    bt audio_start_key
    mov r8,r4
    call audio_queue
    add #1,r8
    bra audio_start_fill
    nop
audio_start_key:
    mov.l #AUDIO_NEXT,r1
    mov.l r9,@r1
    mov.l #AUDIO_COPY_BYTES,r1
    mov #0,r0
    mov.l r0,@r1
    mov.l #SCSP,r1
    mov #0,r0
    mov.w r0,@(2,r1)
    mov.w r0,@(4,r1)
    mov.l #35280,r0
    mov.w r0,@(6,r1)
    mov.l #$1821,r0
    mov.w r0,@r1
    mov.l #AUDIO_ACTIVE,r1
    mov #1,r0
    mov.l r0,@r1
    mov.l #AUDIO_STARTS,r1
    mov.l @r1,r0
    add #1,r0
    mov.l r0,@r1
audio_start_done:
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
