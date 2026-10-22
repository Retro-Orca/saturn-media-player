; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

cd_init:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l #cd_init_commands,r8
    mov #7,r9
cd_init_loop:
    mov.w @r8+,r4
    extu.w r4,r4
    mov.w @r8+,r5
    mov.w @r8+,r6
    mov.w @r8+,r7
    mov.w @r8+,r3
    call cd_command
    tst r0,r0
    bt cd_init_done
    dt r9
    bf cd_init_loop
cd_init_done:
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    align 4
cd_init_commands:
    dc.w $7500,0,0,0,0
    dc.w $0401,0,0,$040F,$40
    dc.w $0600,0,0,0,0
    dc.w $48FC,0,0,0,$40
    dc.w $6000,0,0,0,$40
    dc.w $4400,0,0,0,$40
    dc.w $4603,0,0,0,$40
    ltorg

cd_stop:
    sts.l pr,@-r15
    mov.l #CD_SECTORS_LEFT,r1
    mov #0,r0
    mov.l r0,@r1
    mov.l #$7500,r4
    mov #0,r5
    mov #0,r6
    mov #0,r7
    mov #0,r3
    call cd_command
    lds.l @r15+,pr
    rts
    nop
    ltorg
