; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; 音声も1KiBずつ補充し、1フレームに大きなコピーを集中させない
audio_pump:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l r10,@-r15
    mov.l r11,@-r15
    profile_start 212
    mov.l #AUDIO_ACTIVE,r1
    mov.l @r1,r0
    tst r0,r0
    bt audio_pump_done
    mov.l #AUDIO_NEXT,r1
    mov.l @r1,r8
    mov.l @(FILL_GROUP,r14),r0
    cmp/hs r0,r8
    bt audio_pump_done
    mov r8,r4
    call group_address
    mov r0,r4
    mov.l #SOUND_OFFSET,r0
    add r0,r4
    mov.l #AUDIO_COPY_BYTES,r1
    mov.l @r1,r9
    add r9,r4
    mov.l #AUDIO_ORIGIN,r1
    mov.l @r1,r0
    mov r8,r10
    sub r0,r10
    mov r10,r0
    and #7,r0
    mov.l #SOUND_SIZE,r1
    mulu.w r1,r0
    sts macl,r10
    mov.l #AUDIO_SKIP,r1
    mov.l @r1,r0
    sub r0,r10
    add r9,r10
    cmp/pz r10
    bt audio_pump_high
    mov.l #AUDIO_RING_SIZE,r0
    add r0,r10
audio_pump_high:
    mov.l #AUDIO_RING_SIZE,r0
    cmp/hs r0,r10
    bf audio_pump_count
    sub r0,r10
audio_pump_count:
    mov.l #1024,r11
    mov.l #SOUND_SIZE,r0
    sub r9,r0
    cmp/hi r0,r11
    bf audio_pump_ring_end
    mov r0,r11
audio_pump_ring_end:
    mov.l #AUDIO_RING_SIZE,r0
    sub r10,r0
    cmp/hi r0,r11
    bf audio_pump_copy
    mov r0,r11
audio_pump_copy:
    mov.l #SOUND_RAM+AUDIO_RING,r5
    add r10,r5
    mov r11,r6
    call copy_words
    mov.l #SOUND_RAM+AUDIO_RING,r1
    mov.w @r1,r0
    mov.l #SOUND_RAM+AUDIO_RING+AUDIO_RING_SIZE,r1
    mov.w r0,@r1
    add r11,r9
    mov.l #SOUND_SIZE,r0
    cmp/eq r0,r9
    bf audio_pump_store
    mov #0,r9
    add #1,r8
    mov.l #AUDIO_NEXT,r1
    mov.l r8,@r1
    mov.l #AUDIO_QUEUED,r1
    mov.l @r1,r0
    add #1,r0
    mov.l r0,@r1
audio_pump_store:
    mov.l #AUDIO_COPY_BYTES,r1
    mov.l r9,@r1
audio_pump_done:
    profile_end 212,184
    mov.l @r15+,r11
    mov.l @r15+,r10
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
