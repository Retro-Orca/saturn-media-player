/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>

/* 同じ画素を再利用して浮いた容量を細かい辞書へ配分する */
static void encode_maps(uint8_t *group, const uint16_t *desired, const uint16_t *masks)
{
    unsigned position = CODEBOOK_BYTES + GROUP_FRAMES * 4;
    for (unsigned frame = 0; frame < GROUP_FRAMES; ++frame) {
        unsigned start = position;
        put_be32(group + CODEBOOK_BYTES + frame * 4, start);
        memset(group + start, 0, FRAME_BANK_BYTES);
        position += FRAME_BANK_BYTES;
        for (unsigned cell = 0; cell < FRAME_PATCHES; ++cell) {
            unsigned patch = frame * FRAME_PATCHES + cell;
            if (!(masks[cell] & (1u << frame))) continue;
            if (position + 5 > SOUND_OFFSET) fail("Video packet budget exceeded");
            group[start + cell / 8] |= (uint8_t)(0x80 >> (cell % 8));
            uint64_t packed = 0;
            for (unsigned q = 0; q < 4; ++q) packed = packed << 9 | desired[patch * 4 + q];
            for (unsigned b = 0; b < 5; ++b) group[position + b] = (uint8_t)(packed >> ((4 - b) * 8));
            position += 5;
        }
    }
}


void encode_group(FILE *output, const uint8_t *rgb, const uint8_t *audio)
{
    const int patches = GROUP_FRAMES * FRAME_PATCHES;
    uint8_t *tiles = allocate((size_t)patches * 4 * TILE_COMPONENTS);
    uint8_t *centers = allocate(CODEBOOK_ENTRIES * TILE_COMPONENTS);
    uint8_t *group = allocate(GROUP_BYTES);
    int index = 0;
    for (int frame = 0; frame < GROUP_FRAMES; ++frame)
        for (int ty = 0; ty < VIDEO_HEIGHT; ty += 8)
            for (int tx = 0; tx < VIDEO_WIDTH; tx += 8)
                for (int quadrant = 0; quadrant < 4; ++quadrant) {
                    uint8_t *tile = tiles + index++ * TILE_COMPONENTS;
                    for (int y = 0; y < 4; ++y)
                        for (int x = 0; x < 4; ++x)
                            for (int color = 0; color < 3; ++color) {
                                size_t source = ((size_t)frame * VIDEO_WIDTH * VIDEO_HEIGHT + (ty + quadrant / 2 * 4 + y) * VIDEO_WIDTH + tx + quadrant % 2 * 4 + x) * 3 + color;
                                *tile++ = (uint8_t)(rgb[source] >> 3);
                            }
                }
    train_codebook(tiles, patches * 4, centers);
    for (int entry = 0; entry < CODEBOOK_ENTRIES; ++entry)
        for (int pixel = 0; pixel < 16; ++pixel) {
            const uint8_t *color = centers + entry * TILE_COMPONENTS + pixel * 3;
            uint16_t value = (uint16_t)(0x8000 | color[0] | color[1] << 5 | color[2] << 10);
            put_be16(group + entry * TILE_BYTES + pixel * 2, value);
        }
    uint16_t *desired = allocate((size_t)patches * 4 * sizeof(*desired));
    for (int tile = 0; tile < patches * 4; ++tile) {
        unsigned index = nearest_codeword(tiles + tile * TILE_COMPONENTS, centers);
        desired[tile] = (uint16_t)index;
    }
    uint16_t masks[FRAME_PATCHES];
    select_updates(tiles, centers, desired, masks);
    encode_maps(group, desired, masks);
    memcpy(group + SOUND_OFFSET, audio, AUDIO_BYTES);
    free(desired);
    write_all(output, group, GROUP_BYTES);
    free(group); free(centers); free(tiles);
}
