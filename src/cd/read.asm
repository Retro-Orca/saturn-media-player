; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

; r4のLBAからr5セクターを連続で読み始める
cd_begin:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov r4,r8
    mov r5,r9
    mov.l #CD_SECTORS_LEFT,r1
    mov.l r9,@r1
    mov.l #$4800,r4
    mov #0,r5
    mov #0,r6
    mov #0,r7
    mov #$40,r3
    call cd_command
    tst r0,r0
    bt cd_begin_done
    mov.l #$3000,r4
    mov #0,r3
    mov #$40,r3
    call cd_command
    tst r0,r0
    bt cd_begin_done
    mov.l #150,r0
    add r0,r8
    mov r8,r4
    shlr16 r4
    mov.l #$1080,r0
    or r0,r4
    mov r8,r5
    mov r9,r6
    shlr16 r6
    mov.l #$80,r0
    or r0,r6
    mov r9,r7
    mov #0,r3
    call cd_command
cd_begin_done:
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg

; r4の転送先へ最大r5セクターを取得する 未到着なら即座に0を返す
cd_read_available:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov r4,r8
    mov r5,r9
    mov.l #$5100,r4
    mov #0,r5
    mov #0,r6
    mov #0,r7
    mov #0,r3
    call cd_command
    tst r0,r0
    bt cd_read_done
    mov.l #CD+$24,r1
    mov.w @r1,r0
    extu.w r0,r0
    tst r0,r0
    bt cd_read_done
    cmp/hs r9,r0
    bt cd_read_count
    mov r0,r9
cd_read_count:
    mov.l #$6300,r4
    mov #0,r5
    mov #0,r6
    mov r9,r7
    mov #2,r3
    call cd_command
    tst r0,r0
    bt cd_read_done
    mov r9,r2
    shll8 r2
    shll2 r2
    mov.l #CD_DATA,r1
cd_data_words:
    mov.w @r1,r0
    mov.w r0,@r8
    add #2,r8
    dt r2
    bf cd_data_words
    mov.l #$0600,r4
    mov #0,r5
    mov #0,r6
    mov #0,r7
    mov #0,r3
    call cd_command
    tst r0,r0
    bt cd_read_done
    mov.l #CD_SECTORS_LEFT,r1
    mov.l @r1,r0
    sub r9,r0
    mov.l r0,@r1
    mov r9,r0
cd_read_done:
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg
