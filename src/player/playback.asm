; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

player_open:
    sts.l pr,@-r15
    mov.l @(SELECTION,r14),r0
    shll2 r0
    shll2 r0
    mov.l #movie_catalog,r1
    add r0,r1
    mov.l r1,@(MOVIE,r14)
    mov.l @(4,r1),r0
    mov.l r0,@(MOVIE_GROUPS,r14)
    mov.l @(8,r1),r0
    mov.l r0,@(MOVIE_FRAMES,r14)
    mov #0,r4
    call player_seek
    lds.l @r15+,pr
    rts
    nop
    ltorg

; r4は12フレーム単位の再開位置 各グループは独立して復元できる
player_seek:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov r4,r8
    call audio_stop
    call cd_stop
    mov.l #CD_ERROR,r1
    mov #0,r0
    mov.l r0,@r1
    mov.l r8,@(PLAY_GROUP,r14)
    mov.l r8,@(FILL_GROUP,r14)
    mov.l r0,@(PLAY_FRAME,r14)
    mov.l r0,@(FILL_SECTOR,r14)
    mov.l r0,@(MAP_BANK,r14)
    mov #12,r0
    mulu.w r8,r0
    sts macl,r0
    mov.l r0,@(FRAME_NUMBER,r14)
    mov.l #PREPARED_GROUP,r1
    mov #-1,r0
    mov.l r0,@r1
    mov.l #NEXT_PREPARED,r1
    mov.l r0,@r1
    mov.l #NEXT_COPY_GROUP,r1
    mov.l r0,@r1
    mov #4,r0
    mov.l r0,@(MODE,r14)
    call loading_draw
    mov.l @(MOVIE,r14),r1
    mov.l @r1,r4
    mov #27,r0
    mulu.w r8,r0
    sts macl,r0
    add r0,r4
    mov.l @(MOVIE_GROUPS,r14),r5
    sub r8,r5
    mov #27,r0
    mulu.w r5,r0
    sts macl,r5
    call cd_begin
    tst r0,r0
    bf player_seek_done
    call player_error
player_seek_done:
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg

player_buffering_tick:
    sts.l pr,@-r15
    mov.l #CD_ERROR,r1
    mov.l @r1,r0
    tst r0,r0
    bf player_buffer_error
    mov.l @(MOVIE_GROUPS,r14),r1
    mov.l @(PLAY_GROUP,r14),r0
    sub r0,r1
    mov #4,r2
    cmp/hi r2,r1
    bf player_buffer_target
    mov r2,r1
player_buffer_target:
    mov.l @(FILL_GROUP,r14),r2
    sub r0,r2
    cmp/hs r1,r2
    bf player_buffer_done
    call video_tile_mode
    mov.l @(PLAY_GROUP,r14),r4
    mov r4,r0
    and #1,r0
    mov.l r0,@(DICT_BANK,r14)
    call group_address
    mov r0,r4
    call video_dictionary
    mov.l @(PLAY_GROUP,r14),r0
    mov.l #PREPARED_GROUP,r1
    mov.l r0,@r1
    call audio_start
    call video_wait_vblank
    mov #1,r0
    mov.l r0,@(MODE,r14)
    bra player_buffer_done
    nop
player_buffer_error:
    call player_error
player_buffer_done:
    lds.l @r15+,pr
    rts
    nop
    ltorg

player_tick:
    sts.l pr,@-r15
    mov.l r8,@-r15
    mov.l #CD_ERROR,r1
    mov.l @r1,r0
    tst r0,r0
    bt player_tick_check_end
    call player_error
    bra player_tick_done
    nop
player_tick_check_end:
    mov.l @(FRAME_NUMBER,r14),r0
    mov.l @(MOVIE_FRAMES,r14),r1
    cmp/hs r1,r0
    bf player_tick_ready
    call player_to_menu
    bra player_tick_done
    nop
player_tick_ready:
    mov.l @(PLAY_GROUP,r14),r4
    mov.l @(FILL_GROUP,r14),r0
    cmp/hs r0,r4
    bf player_tick_address
    call audio_stop
    mov.l #UNDERRUNS,r1
    mov.l @r1,r0
    add #1,r0
    mov.l r0,@r1
    call video_wait_vblank
    bra player_tick_done
    nop
player_tick_address:
    call group_address
    mov r0,r8
    mov.l #PREPARED_GROUP,r1
    mov.l @r1,r0
    mov.l @(PLAY_GROUP,r14),r2
    cmp/eq r2,r0
    bt player_tick_frame
    mov.l r2,@r1
    mov r2,r0
    and #1,r0
    mov.l r0,@(DICT_BANK,r14)
    mov.l #NEXT_PREPARED,r1
    mov.l @r1,r0
    cmp/eq r2,r0
    bt player_tick_frame
    mov r8,r4
    call video_dictionary
player_tick_frame:
    mov.l @(PLAY_FRAME,r14),r0
    shll2 r0
    mov.l #FRAME_DIRECTORY,r1
    add r8,r1
    add r0,r1
    mov.l @r1,r4
    add r8,r4
    call video_frame
    call audio_pump
    call player_prefetch
    call video_wait_vblank
    call video_present
    mov.l @(FRAME_NUMBER,r14),r0
    add #1,r0
    mov.l r0,@(FRAME_NUMBER,r14)
    mov.l @(PLAY_FRAME,r14),r0
    add #1,r0
    cmp/eq #12,r0
    bf player_tick_store_frame
    mov.l @(PLAY_GROUP,r14),r0
    add #1,r0
    mov.l r0,@(PLAY_GROUP,r14)
    mov #0,r0
player_tick_store_frame:
    mov.l r0,@(PLAY_FRAME,r14)
player_tick_done:
    mov.l @r15+,r8
    lds.l @r15+,pr
    rts
    nop
    ltorg

player_error:
    sts.l pr,@-r15
    call audio_stop
    mov #3,r0
    mov.l r0,@(MODE,r14)
    mov #1,r0
    mov.l r0,@(DIRTY_MENU,r14)
    lds.l @r15+,pr
    rts
    nop
    ltorg

player_to_menu:
    sts.l pr,@-r15
    call audio_stop
    call cd_stop
    mov #0,r0
    mov.l r0,@(MODE,r14)
    mov #1,r0
    mov.l r0,@(DIRTY_MENU,r14)
    lds.l @r15+,pr
    rts
    nop
    ltorg
