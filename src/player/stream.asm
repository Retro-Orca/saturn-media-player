; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

stream_pump:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l #CD_ERROR,r1
    mov.l @r1,r0
    tst r0,r0
    bf stream_pump_done
    mov.l @(MODE,r14),r0
    tst r0,r0
    bt stream_pump_done
    cmp/eq #3,r0
    bt stream_pump_done
    mov.l @(FILL_GROUP,r14),r4
    mov.l @(MOVIE_GROUPS,r14),r0
    cmp/hs r0,r4
    bt stream_pump_done
    mov.l @(PLAY_GROUP,r14),r0
    add #8,r0
    cmp/hs r0,r4
    bt stream_pump_done
    call group_address
    mov r0,r4
    mov.l @(FILL_SECTOR,r14),r0
    mov r0,r8
    shll8 r0
    shll2 r0
    shll r0
    add r0,r4
    mov #27,r5
    sub r8,r5
    mov #4,r0
    cmp/hi r0,r5
    bf stream_pump_read
    mov r0,r5
stream_pump_read:
    call cd_read_available
    add r8,r0
    cmp/eq #27,r0
    bf stream_pump_partial
    mov #0,r0
    mov.l r0,@(FILL_SECTOR,r14)
    mov.l @(FILL_GROUP,r14),r0
    add #1,r0
    mov.l r0,@(FILL_GROUP,r14)
    bra stream_pump_done
    nop
stream_pump_partial:
    mov.l r0,@(FILL_SECTOR,r14)
stream_pump_done:
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
