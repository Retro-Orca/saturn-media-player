/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../tools/common/common.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static unsigned ramp_error(unsigned mask)
{
    unsigned last = 0, error = 0;
    for (unsigned frame = 0; frame < GROUP_FRAMES; ++frame) {
        if (mask & (1u << frame)) last = frame;
        error += (frame - last) * (frame - last) * 768;
    }
    return error;
}

void test_temporal_budget(void)
{
    const unsigned tile_count = GROUP_FRAMES * FRAME_PATCHES * 4;
    uint8_t *tiles = allocate((size_t)tile_count * TILE_COMPONENTS);
    uint16_t *desired = allocate(tile_count * sizeof(*desired));
    uint8_t centers[CODEBOOK_ENTRIES * TILE_COMPONENTS] = {0};
    for (unsigned frame = 0; frame < GROUP_FRAMES; ++frame) {
        memset(centers + frame * TILE_COMPONENTS, (int)frame * 2, TILE_COMPONENTS);
        for (unsigned tile = 0; tile < FRAME_PATCHES * 4; ++tile) {
            unsigned index = frame * FRAME_PATCHES * 4 + tile;
            memset(tiles + index * TILE_COMPONENTS, (int)frame * 2, TILE_COMPONENTS);
            desired[index] = (uint16_t)frame;
        }
    }
    uint16_t masks[FRAME_PATCHES];
    select_updates(tiles, centers, desired, masks);
    const unsigned budget = (SOUND_OFFSET - CODEBOOK_BYTES - GROUP_FRAMES * (4 + FRAME_BANK_BYTES)) / 5;
    uint64_t error = 0;
    unsigned updates = 0;
    for (unsigned cell = 0; cell < FRAME_PATCHES; ++cell) {
        if (!(masks[cell] & 1) || masks[cell] >> GROUP_FRAMES) fail("Invalid temporal update mask");
        for (unsigned frame = 0; frame < GROUP_FRAMES; ++frame) updates += !!(masks[cell] & (1u << frame));
        error += ramp_error(masks[cell]);
    }
    /* 1セルの全2048通りを列挙し、既知の有効な配分より誤差が増えないことを確認する */
    unsigned oracle[GROUP_FRAMES + 1];
    for (unsigned i = 0; i <= GROUP_FRAMES; ++i) oracle[i] = UINT_MAX;
    for (unsigned mask = 1; mask < 1u << GROUP_FRAMES; mask += 2) {
        unsigned count = 0;
        for (unsigned frame = 0; frame < GROUP_FRAMES; ++frame) count += !!(mask & (1u << frame));
        unsigned value = ramp_error(mask);
        if (value < oracle[count]) oracle[count] = value;
    }
    unsigned base = budget / FRAME_PATCHES, extra = budget % FRAME_PATCHES;
    uint64_t feasible = (uint64_t)(FRAME_PATCHES - extra) * oracle[base] + (uint64_t)extra * oracle[base + 1];
    if (updates != budget || error > feasible) fail("Temporal allocation lost detail or exceeded its quota");
    free(desired); free(tiles);
    puts("PASS: temporal allocation uses the full quota and beats the exhaustive per-cell reference allocation");
}
