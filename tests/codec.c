/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../tools/common/common.h"
#include <stdlib.h>
#include <string.h>

static void check_packet_budget(uint8_t *rgb, uint8_t *group, const uint8_t *sound)
{
    uint32_t random = 1;
    for (size_t i = 0; i < (size_t)VIDEO_WIDTH * VIDEO_HEIGHT * 3 * GROUP_FRAMES; ++i) {
        random = random * 1664525u + 1013904223u;
        rgb[i] = (uint8_t)(random >> 24);
    }
    FILE *file = open_file("build/budget-test.bin", "wb");
    encode_group(file, rgb, sound);
    fclose(file);
    file = open_file("build/budget-test.bin", "rb");
    read_all(file, group, GROUP_BYTES);
    fclose(file);
    unsigned end = CODEBOOK_BYTES + GROUP_FRAMES * 4, updates = 0;
    for (unsigned frame = 0; frame < GROUP_FRAMES; ++frame) {
        unsigned start = get_be32(group + CODEBOOK_BYTES + frame * 4);
        if (start != end || start + FRAME_BANK_BYTES > SOUND_OFFSET) fail("Invalid frame directory");
        end += FRAME_BANK_BYTES;
        for (unsigned cell = 0; cell < FRAME_PATCHES; ++cell) {
            int changed = group[start + cell / 8] & (0x80 >> (cell % 8));
            if (!frame && !changed) fail("First frame depends on the previous group");
            if (!changed) continue;
            if (end + 5 > SOUND_OFFSET || group[end] & 0xF0) fail("Video packet exceeds the CD budget");
            uint64_t packed = 0;
            for (unsigned b = 0; b < 5; ++b) packed = packed << 8 | group[end + b];
            for (unsigned q = 0; q < 4; ++q)
                if (((packed >> (q * 9)) & 511) >= CODEBOOK_ENTRIES) fail("Invalid dictionary reference");
            end += 5;
            ++updates;
        }
    }
    if (updates > 6492 || memcmp(group + SOUND_OFFSET, sound, AUDIO_BYTES)) fail("Video overwrote audio");
    puts("PASS: incompressible motion stays within 27 sectors with valid references and independent group starts");
}

int run_self_tests(void)
{
    const size_t frame_size = VIDEO_WIDTH * VIDEO_HEIGHT * 3;
    uint8_t *rgb = allocate(frame_size * GROUP_FRAMES);
    /* 端のタイルと色の並びを識別できる入力で辞書と番号の往復を検証する */
    for (int frame = 0; frame < GROUP_FRAMES; ++frame)
        for (int y = 0; y < VIDEO_HEIGHT; ++y)
            for (int x = 0; x < VIDEO_WIDTH; ++x) {
                size_t p = ((size_t)frame * VIDEO_WIDTH * VIDEO_HEIGHT + y * VIDEO_WIDTH + x) * 3;
                rgb[p] = (uint8_t)((x / 4 % 4) * 64);
                rgb[p + 1] = (uint8_t)((y / 4 % 4) * 64);
                rgb[p + 2] = (uint8_t)((frame / 4 % 4) * 64);
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
        for (int tile = 0; tile < FRAME_PATCHES; ++tile) {
            uint64_t indices = 0;
            for (int previous = 0; previous <= frame; ++previous) {
                const uint8_t *map = group + get_be32(group + CODEBOOK_BYTES + previous * 4);
                unsigned position = FRAME_BANK_BYTES;
                for (int cell = 0; cell <= tile; ++cell) {
                    if (!(map[cell / 8] & (0x80 >> (cell % 8)))) continue;
                    if (cell == tile) {
                        indices = 0;
                        for (unsigned b = 0; b < 5; ++b) indices = indices << 8 | map[position + b];
                    }
                    position += 5;
                }
            }
            for (int pixel = 0; pixel < 64; ++pixel) {
                int x = tile % 40 * 8 + pixel % 8;
                int y = tile / 40 * 8 + pixel / 8;
                size_t source = ((size_t)frame * VIDEO_WIDTH * VIDEO_HEIGHT + y * VIDEO_WIDTH + x) * 3;
                unsigned expected = 0x8000 | rgb[source] >> 3 | (rgb[source + 1] >> 3) << 5 | (rgb[source + 2] >> 3) << 10;
                unsigned quadrant = (pixel / 8 / 4) * 2 + pixel % 8 / 4;
                unsigned index = (unsigned)((indices >> (27 - quadrant * 9)) & 511);
                if (index >= CODEBOOK_ENTRIES) fail("Invalid codeword index");
                unsigned small_pixel = pixel / 8 % 4 * 4 + pixel % 4;
                const uint8_t *color = group + index * TILE_BYTES + small_pixel * 2;
                unsigned actual = color[0] << 8 | color[1];
                if (expected != actual) fail("Codec round-trip failed at frame %d tile %d", frame, tile);
            }
        }
    if (memcmp(group + SOUND_OFFSET, sound, AUDIO_BYTES)) fail("Audio payload changed");
    unsigned end = get_be32(group + CODEBOOK_BYTES + (GROUP_FRAMES - 1) * 4);
    unsigned count = 0;
    for (int cell = 0; cell < FRAME_PATCHES; ++cell) if (group[end + cell / 8] & (0x80 >> (cell % 8))) ++count;
    end += FRAME_BANK_BYTES + count * 5;
    for (unsigned i = end; i < SOUND_OFFSET; ++i) if (group[i]) fail("Nonzero sector padding");
    check_packet_budget(rgb, group, sound);
    free(group); free(rgb);
    puts("PASS: SVT2 4x4 RGB555, retained pixels, quadrant order, PCM bytes, sector padding, header");
    return 0;
}
