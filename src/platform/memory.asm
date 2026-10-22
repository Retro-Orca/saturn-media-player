; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; r4からr5へr6バイトを転送する 両端と長さはワード境界にそろえる
copy_words:
    tst r6,r6
    bt copy_words_done
    mov r4,r0
    or r5,r0
    or r6,r0
    tst #3,r0
    bf copy_words_short
    shlr2 r6
copy_longs_loop:
    mov.l @r4+,r0
    mov.l r0,@r5
    add #4,r5
    dt r6
    bf copy_longs_loop
    bra copy_words_done
    nop
copy_words_short:
    shlr r6
copy_words_loop:
    mov.w @r4+,r0
    mov.w r0,@r5
    add #2,r5
    dt r6
    bf copy_words_loop
copy_words_done:
    rts
    nop

; r4で指定したグループの循環バッファをr0に返す
group_address:
    mov r4,r0
    and #7,r0
    mov.l #GROUP_SIZE,r1
    mulu.w r1,r0
    sts macl,r0
    mov.l #STREAM_RAM,r1
    add r1,r0
    rts
    nop
    ltorg
