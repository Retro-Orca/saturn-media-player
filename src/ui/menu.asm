; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

menu_draw:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l r9,@-r15
    mov.l r10,@-r15
    mov.l r11,@-r15
    call video_bitmap_mode
    call bitmap_clear
    mov.l #menu_heading,r4
    mov #16,r5
    mov #8,r6
    mov.l #$FFE0,r3
    mov.l #$8000,r2
    call ui_text
    mov.l #menu_start_hint,r4
    mov #16,r5
    mov #28,r6
    mov.l #$FFFF,r3
    mov.l #$8000,r2
    call ui_text
    mov.l @(SELECTION,r14),r11
    mov r11,r0
    and #$F8,r0
    mov r0,r8
    mov #52,r9
    mov #8,r10
menu_item_loop:
    mov.l #MOVIE_COUNT,r0
    cmp/hs r0,r8
    bt menu_footer
    mov r8,r0
    shll2 r0
    shll2 r0
    mov.l #movie_catalog,r1
    add r0,r1
    mov.l @(12,r1),r4
    mov #24,r5
    mov r9,r6
    mov #32,r7
    mov.l #$FFFF,r3
    mov.l #$8000,r2
    cmp/eq r11,r8
    bf menu_item_draw
    mov.l #$83E0,r3
    mov.l #$8C42,r2
menu_item_draw:
    call bitmap_mask
    add #1,r8
    add #16,r9
    dt r10
    bf menu_item_loop
menu_footer:
    mov.l #menu_button_hint,r4
    mov #16,r5
    mov.l #184,r6
    mov.l #$FFFF,r3
    mov.l #$8000,r2
    call ui_text
    mov.l #menu_seek_hint,r4
    mov #16,r5
    mov.l #202,r6
    mov.l #$FFE0,r3
    mov.l #$8000,r2
    call ui_text
    call ui_show
    mov #0,r0
    mov.l r0,@(DIRTY_MENU,r14)
    mov.l @r15+,r11
    mov.l @r15+,r10
    mov.l @r15+,r9
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg

loading_draw:
    sts.l pr,@-r15
    call video_bitmap_mode
    call bitmap_clear
    mov.l #loading_text,r4
    mov #64,r5
    mov #96,r6
    mov.l #$FFE0,r3
    mov.l #$8000,r2
    call ui_text
    call ui_show
    lds.l @r15+,pr
    rts
    nop
    ltorg

error_draw:
    sts.l pr,@-r15
    mov.l @(DIRTY_MENU,r14),r0
    tst r0,r0
    bt error_draw_done
    call video_bitmap_mode
    call bitmap_clear
    mov.l #error_text,r4
    mov #64,r5
    mov #88,r6
    mov.l #$801F,r3
    mov.l #$8000,r2
    call ui_text
    mov.l #error_hint,r4
    mov #64,r5
    mov #112,r6
    mov.l #$FFFF,r3
    mov.l #$8000,r2
    call ui_text
    call ui_show
    mov #0,r0
    mov.l r0,@(DIRTY_MENU,r14)
error_draw_done:
    lds.l @r15+,pr
    rts
    nop
    ltorg
menu_heading:
    dc.b "SATURN VIDEO PLAYER",0
menu_start_hint:
    dc.b "UP/DOWN SELECT   A/START PLAY",0
menu_button_hint:
    dc.b "X LIST  Y PAUSE  Z PLAY/PAUSE",0
menu_seek_hint:
    dc.b "L +5 SEC       R +30 SEC",0
loading_text:
    dc.b "BUFFERING...  X: LIST",0
error_text:
    dc.b "CD READ ERROR",0
error_hint:
    dc.b "X: VIDEO LIST",0
    align 4
