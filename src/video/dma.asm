; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

video_upload:
    mov.l #$25FE0000,r1
    mov.l #FRAME_RAM,r0
    mov.l r0,@r1
    mov.l @(MAP_BANK,r14),r0
    shll16 r0
    shll2 r0
    mov.l #$05E00000,r2
    add r2,r0
    mov.l r0,@(4,r1)
    mov.l #143360,r0
    mov.l r0,@(8,r1)
    mov.l #$101,r0
    mov.l r0,@(12,r1)
    mov #7,r0
    mov.l r0,@(20,r1)
    mov.l #$101,r0
    mov.l r0,@(16,r1)
    mov.l #$25FE007C,r1
    mov.l #$100000,r2
video_upload_wait:
    mov.l @r1,r0
    tst #$30,r0
    bt video_upload_done
    dt r2
    bf video_upload_wait
    mov.l #CD_ERROR,r1
    mov #-2,r0
    mov.l r0,@r1
video_upload_done:
    rts
    nop
    ltorg
