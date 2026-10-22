; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

video_init:
    mov.l #VDP2,r1
    mov #0,r0
    mov.l #$90,r2
video_clear_registers:
    mov.w r0,@r1
    add #2,r1
    dt r2
    bf video_clear_registers
    mov.l #$25D00004,r1
    mov.w r0,@r1
    mov.l #$25D0000C,r1
    mov.w r0,@r1
    mov.l #VDP2+$0E,r1
    mov.l #$0300,r0
    mov.w r0,@r1
    mov.l #VDP2+$78,r1
    mov #1,r0
    mov.w r0,@r1
    add #4,r1
    mov.w r0,@r1
    mov.l #VDP2+$F8,r1
    mov.w r0,@r1
    mov.l #VDP2+$AC,r1
    mov #3,r0
    mov.w r0,@r1
    add #2,r1
    mov.l #$FFFF,r0
    mov.w r0,@r1
    mov.l #VRAM+$7FFFE,r1
    mov.l #$8000,r0
    mov.w r0,@r1
    mov.l #VDP2,r1
    mov.w r0,@r1
    rts
    nop
    ltorg

video_wait_vblank:
    sts.l pr,@-r15
    mov.l #VDP2+$04,r1
video_wait_active:
    mov.w @r1,r0
    tst #8,r0
    bf video_wait_active
video_wait_blank:
    mov.w @r1,r0
    tst #8,r0
    bt video_wait_blank
    mov.l @(VBLANK_COUNT,r14),r0
    add #1,r0
    mov.l r0,@(VBLANK_COUNT,r14)
    call video_tick_time
    lds.l @r15+,pr
    rts
    nop
    ltorg

video_bitmap_mode:
    mov.l #VDP2+$20,r1
    mov #0,r0
    mov.w r0,@r1
    mov.l #VDP2+$28,r1
    mov #$32,r0
    mov.w r0,@r1
    mov.l #VDP2+$3C,r1
    mov #0,r0
    mov.w r0,@r1
    mov.l #VDP2+$10,r1
    mov #4,r2
video_bitmap_cycles:
    mov.l #$4444,r0
    mov.w r0,@r1
    add #2,r1
    mov.l #$EEEE,r0
    mov.w r0,@r1
    add #2,r1
    dt r2
    bf video_bitmap_cycles
    rts
    nop
    ltorg

video_tile_mode:
    sts.l pr,@-r15
    mov.l #VDP2+$20,r1
    mov #0,r0
    mov.w r0,@r1
    mov.l #VDP2+$28,r1
    mov #$30,r0
    mov.w r0,@r1
    mov.l #VDP2+$30,r1
    mov #0,r0
    mov.w r0,@r1
    mov.l #VDP2+$3C,r1
    mov.w r0,@r1
    mov.l #VDP2+$10,r1
    mov #8,r2
    mov.l #$EEEE,r0
video_tile_cycles:
    mov.w r0,@r1
    add #2,r1
    dt r2
    bf video_tile_cycles
    mov.l #VRAM+$30000,r4
    mov #0,r5
    call video_map_init
    mov.l #VRAM+$70000,r4
    mov.l #8192,r5
    call video_map_init
    lds.l @r15+,pr
    rts
    nop
    ltorg
