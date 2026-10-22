/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../tools/common/common.h"
#include <stdlib.h>
#include <string.h>

int run_self_tests(void)
{
    const size_t frame_size = VIDEO_WIDTH * VIDEO_HEIGHT * 3;
    uint8_t *rgb = allocate(frame_size * GROUP_FRAMES);
    /* 端のタイルと色の並びを識別できる入力で辞書と番号の往復を検証する */
    for (int frame = 0; frame < GROUP_FRAMES; ++frame)
        for (int y = 0; y < VIDEO_HEIGHT; ++y)
            for (int x = 0; x < VIDEO_WIDTH; ++x) {
                size_t p = ((size_t)frame * VIDEO_WIDTH * VIDEO_HEIGHT + y * VIDEO_WIDTH + x) * 3;
                rgb[p] = (uint8_t)((x / 8 % 4) * 64);
                rgb[p + 1] = (uint8_t)((y / 8 % 4) * 64);
                rgb[p + 2] = (uint8_t)((frame % 4) * 64);
            }
    uint8_t sound[AUDIO_BYTES];
    for (int i = 0; i < AUDIO_BYTES; ++i) sound[i] = (uint8_t)(i * 17);
    FILE *file = open_file("build/codec-test.svt", "wb");
    write_video_header(file, GROUP_FRAMES, 1);
    encode_group(file, rgb, sound);
    fclose(file);
    Movie movie = {0}; strcpy(movie.encoded, "build/codec-test.svt");
    read_video_header(&movie);
    file = open_file(movie.encoded, "rb");
    _fseeki64(file, SECTOR_BYTES, SEEK_SET);
    uint8_t *group = allocate(GROUP_BYTES);
    read_all(file, group, GROUP_BYTES);
    fclose(file);
    for (int frame = 0; frame < GROUP_FRAMES; ++frame)
        for (int tile = 0; tile < FRAME_BYTES; ++tile) {
            unsigned index = group[CODEBOOK_BYTES + frame * FRAME_BYTES + tile];
            for (int pixel = 0; pixel < 64; ++pixel) {
                int x = tile % 40 * 8 + pixel % 8;
                int y = tile / 40 * 8 + pixel / 8;
                size_t source = ((size_t)frame * VIDEO_WIDTH * VIDEO_HEIGHT + y * VIDEO_WIDTH + x) * 3;
                unsigned expected = 0x8000 | rgb[source] >> 3 | (rgb[source + 1] >> 3) << 5 | (rgb[source + 2] >> 3) << 10;
                const uint8_t *color = group + index * TILE_BYTES + pixel * 2;
                unsigned actual = color[0] << 8 | color[1];
                if (expected != actual) fail("Codec round-trip failed at frame %d tile %d", frame, tile);
            }
        }
    if (memcmp(group + CODEBOOK_BYTES + GROUP_FRAMES * FRAME_BYTES, sound, AUDIO_BYTES)) fail("Audio payload changed");
    for (int i = CODEBOOK_BYTES + GROUP_FRAMES * FRAME_BYTES + AUDIO_BYTES; i < GROUP_BYTES; ++i)
        if (group[i]) fail("Nonzero sector padding");
    free(group); free(rgb);
    puts("PASS: codec RGB555 round-trip, tile order, PCM bytes, sector padding, SVT header");
    return 0;
}
