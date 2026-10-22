; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

audio_init:
    sts.l pr,@-r15
    mov.l #SMPC+$63,r1
    mov.l #$100000,r2
audio_wait_idle:
    mov.b @r1,r0
    tst #1,r0
    bt audio_issue_off
    dt r2
    bf audio_wait_idle
    bra audio_init_error
    nop
audio_issue_off:
    mov #1,r0
    mov.b r0,@r1
    mov.l #SMPC+$1F,r3
    mov #7,r0
    mov.b r0,@r3
    mov.l #$100000,r2
audio_wait_off:
    mov.b @r1,r0
    tst #1,r0
    bt audio_registers
    dt r2
    bf audio_wait_off
audio_init_error:
    mov.l #AUDIO_ERROR,r1
    mov #1,r0
    mov.l r0,@r1
    bra audio_init_done
    nop
audio_registers:
    mov.l #SCSP,r1
    mov.l #512,r2
    mov #0,r0
audio_clear_slots:
    mov.w r0,@r1
    add #2,r1
    dt r2
    bf audio_clear_slots
    mov.l #SCSP+$400,r1
    mov.l #$020F,r0
    mov.w r0,@r1
    mov.l #SCSP,r1
    mov.l #$2000,r0
    mov.w r0,@(2,r1)
    mov #31,r0
    mov.w r0,@(8,r1)
    mov.w r0,@(10,r1)
    mov.l #$73FA,r0
    mov.w r0,@(16,r1)
    mov.l #$E000,r0
    mov.w r0,@(22,r1)
audio_init_done:
    lds.l @r15+,pr
    rts
    nop
    ltorg
