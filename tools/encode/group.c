/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>

void encode_group(FILE *output, const uint8_t *rgb, const uint8_t *audio)
{
    const int tile_count = GROUP_FRAMES * FRAME_BYTES;
    uint8_t *tiles = allocate((size_t)tile_count * TILE_COMPONENTS);
    uint8_t *centers = allocate(CODEBOOK_ENTRIES * TILE_COMPONENTS);
    uint8_t *group = allocate(GROUP_BYTES);
    int index = 0;
    for (int frame = 0; frame < GROUP_FRAMES; ++frame)
        for (int ty = 0; ty < VIDEO_HEIGHT; ty += TILE_SIDE)
            for (int tx = 0; tx < VIDEO_WIDTH; tx += TILE_SIDE) {
                uint8_t *tile = tiles + index++ * TILE_COMPONENTS;
                for (int y = 0; y < TILE_SIDE; ++y)
                    for (int x = 0; x < TILE_SIDE; ++x)
                        for (int color = 0; color < 3; ++color) {
                            size_t source = ((size_t)frame * VIDEO_WIDTH * VIDEO_HEIGHT + (ty + y) * VIDEO_WIDTH + tx + x) * 3 + color;
                            *tile++ = (uint8_t)(rgb[source] >> 3);
                        }
            }
    train_codebook(tiles, tile_count, centers);
    for (int entry = 0; entry < CODEBOOK_ENTRIES; ++entry)
        for (int pixel = 0; pixel < 64; ++pixel) {
            const uint8_t *color = centers + entry * TILE_COMPONENTS + pixel * 3;
            uint16_t value = (uint16_t)(0x8000 | color[0] | color[1] << 5 | color[2] << 10);
            put_be16(group + entry * TILE_BYTES + pixel * 2, value);
        }
    for (int tile = 0; tile < tile_count; ++tile)
        group[CODEBOOK_BYTES + tile] = (uint8_t)nearest_codeword(tiles + tile * TILE_COMPONENTS, centers);
    for (int i = 0; i < AUDIO_BYTES; ++i) group[CODEBOOK_BYTES + GROUP_FRAMES * FRAME_BYTES + i] = audio[i];
    write_all(output, group, GROUP_BYTES);
    free(group);
    free(centers);
    free(tiles);
}
