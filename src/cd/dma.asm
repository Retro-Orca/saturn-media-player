; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; CDのデータポートを固定し、高速ワークRAMへSCU DMAで読み出す
cd_dma_read:
    mov.l #$25FE0000,r1
    ; 16ビット用の別名では同じワードを2回読むため32ビット用ポートを使う
    mov.l #$05818000,r0
    mov.l r0,@r1
    mov.l #$07FFFFFF,r0
    and r0,r4
    mov.l r4,@(4,r1)
    shll8 r5
    shll2 r5
    shll r5
    mov.l r5,@(8,r1)
    mov #2,r0
    mov.l r0,@(12,r1)
    mov #7,r0
    mov.l r0,@(20,r1)
    mov.l #$101,r0
    mov.l r0,@(16,r1)
    mov.l #$25FE007C,r1
    mov.l #$100000,r2
cd_dma_wait:
    mov.l @r1,r0
    tst #$30,r0
    bt cd_dma_done
    dt r2
    bf cd_dma_wait
    mov.l #CD_ERROR,r1
    mov #-3,r0
    mov.l r0,@r1
    mov #0,r0
    rts
    nop
cd_dma_done:
    mov #1,r0
    rts
    nop
    ltorg
