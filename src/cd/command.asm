; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; r4からr7がCR1からCR4 r3は待機する完了フラグ r0に成功可否を返す
cd_command:
    mov.l #CD+$08,r1
    mov.l #$200000,r2
cd_wait_ready:
    mov.w @r1,r0
    tst #1,r0
    bf cd_issue
    dt r2
    bf cd_wait_ready
    bra cd_timeout
    nop
cd_issue:
    mov r3,r0
    or #1,r0
    not r0,r0
    mov.w r0,@r1
    mov.l #CD+$18,r1
    mov r4,r0
    mov.w r0,@r1
    add #4,r1
    mov r5,r0
    mov.w r0,@r1
    add #4,r1
    mov r6,r0
    mov.w r0,@r1
    add #4,r1
    mov r7,r0
    mov.w r0,@r1
    mov.l #CD+$08,r1
    mov.l #$200000,r2
cd_wait_complete:
    mov.w @r1,r0
    tst #1,r0
    bf cd_check_response
    dt r2
    bf cd_wait_complete
    bra cd_timeout
    nop
cd_check_response:
    mov.l #CD+$18,r1
    mov.w @r1,r0
    shlr8 r0
    extu.b r0,r0
    mov.l #$FF,r2
    cmp/eq r2,r0
    bt cd_timeout
    tst r3,r3
    bt cd_success
    mov.l #CD+$08,r1
    mov.l #$200000,r2
cd_wait_event:
    mov.w @r1,r0
    and r3,r0
    cmp/eq r3,r0
    bt cd_success
    dt r2
    bf cd_wait_event
cd_timeout:
    mov.l #CD_ERROR,r1
    mov.l r4,@r1
    mov #0,r0
    rts
    nop
cd_success:
    mov #1,r0
    rts
    nop
    ltorg
