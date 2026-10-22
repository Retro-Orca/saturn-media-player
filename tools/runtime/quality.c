/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum { WIDTH = 320, HEIGHT = 224, PIXELS = WIDTH * HEIGHT, GROUP = 55296 };

static unsigned big32(const uint8_t *bytes)
{
    return (unsigned)bytes[0] << 24 | (unsigned)bytes[1] << 16 | (unsigned)bytes[2] << 8 | bytes[3];
}

static void read_exact(FILE *file, void *bytes, size_t length)
{
    if (fread(bytes, 1, length, file) != length) { fputs("Truncated quality input\n", stderr); exit(1); }
}

static void decode(const uint8_t *group, unsigned frame, int version, uint8_t *rgb)
{
    if (version == 2) {
        const uint8_t *directory = group + 12288 + frame * 4;
        unsigned offset = big32(directory);
        if (offset < 12336 || offset > 46476 - 140) { fputs("Invalid frame offset\n", stderr); exit(1); }
        const uint8_t *map = group + offset, *packet = map + 140;
        for (unsigned cell = 0; cell < 1120; ++cell) {
            if (!(map[cell / 8] & (0x80 >> (cell % 8)))) continue;
            if (packet + 5 > group + 46476) { fputs("Invalid video packet\n", stderr); exit(1); }
            uint64_t packed = 0;
            for (unsigned b = 0; b < 5; ++b) packed = packed << 8 | *packet++;
            for (unsigned q = 0; q < 4; ++q) {
                unsigned index = (unsigned)((packed >> (27 - q * 9)) & 511);
                if (index >= 384) { fputs("Invalid dictionary index\n", stderr); exit(1); }
                for (unsigned y = 0; y < 4; ++y)
                    for (unsigned x = 0; x < 4; ++x) {
                        unsigned p = index * 32 + (y * 4 + x) * 2;
                        unsigned color = group[p] << 8 | group[p + 1];
                        unsigned target = ((cell / 40 * 8 + q / 2 * 4 + y) * WIDTH + cell % 40 * 8 + q % 2 * 4 + x) * 3;
                        for (unsigned c = 0; c < 3; ++c) rgb[target + c] = (uint8_t)((color >> (c * 5)) & 31);
                    }
            }
        }
        return;
    }
    const uint8_t *map = group + 32768 + frame * 1120;
    for (unsigned y = 0; y < HEIGHT; ++y)
        for (unsigned x = 0; x < WIDTH; ++x) {
            unsigned cell = y / 8 * 40 + x / 8;
            unsigned offset = map[cell] * 128 + (y % 8 * 8 + x % 8) * 2;
            unsigned color = group[offset] << 8 | group[offset + 1];
            for (unsigned c = 0; c < 3; ++c) rgb[(y * WIDTH + x) * 3 + c] = (uint8_t)((color >> (c * 5)) & 31);
        }
}

static double seam_error(const uint8_t *image, const uint8_t *reference)
{
    double total = 0;
    for (unsigned y = 0; y < HEIGHT; ++y)
        for (unsigned x = 0; x < WIDTH; ++x)
            for (unsigned axis = 0; axis < 2; ++axis) {
                if ((!axis && (!x || x % 8)) || (axis && (!y || y % 8))) continue;
                unsigned p = (y * WIDTH + x) * 3, previous = p - (axis ? WIDTH * 3 : 3);
                for (unsigned c = 0; c < 3; ++c)
                    total += abs((int)image[p + c] - image[previous + c] - (int)reference[p + c] + reference[previous + c]);
            }
    return total;
}

static void picture(const char *path, const uint8_t *rgb)
{
    FILE *file = fopen(path, "wb");
    if (!file) exit(1);
    fprintf(file, "P6\n320 224\n255\n");
    for (unsigned i = 0; i < PIXELS * 3; ++i) fputc(rgb[i] * 255 / 31, file);
    fclose(file);
}

int main(int argc, char **argv)
{
    if (argc != 4) { puts("Usage: quality.exe before.svt after.svt reference.rgb"); return 1; }
    FILE *before = fopen(argv[1], "rb"), *after = fopen(argv[2], "rb"), *reference = fopen(argv[3], "rb");
    if (!before || !after || !reference) return 1;
    uint8_t header[2048], old_group[GROUP], new_group[GROUP];
    read_exact(before, header, sizeof(header));
    int before_version = !memcmp(header, "SVT1", 4) ? 1 : !memcmp(header, "SVT2", 4) ? 2 : 0;
    unsigned expected = big32(header + 16);
    if (!before_version || !expected || big32(header + 8) != WIDTH || big32(header + 12) != HEIGHT) return 1;
    read_exact(after, header, sizeof(header));
    if (memcmp(header, "SVT2", 4) || big32(header + 16) != expected || big32(header + 8) != WIDTH || big32(header + 12) != HEIGHT) return 1;
    uint8_t *raw = malloc(PIXELS * 3), *old_rgb = malloc(PIXELS * 3), *new_rgb = malloc(PIXELS * 3);
    if (!raw || !old_rgb || !new_rgb) return 1;
    double errors[2] = {0}, seams[2] = {0};
    unsigned frames = 0;
    while (frames < expected) {
        read_exact(reference, raw, PIXELS * 3);
        if (!(frames % 12)) { read_exact(before, old_group, GROUP); read_exact(after, new_group, GROUP); }
        decode(old_group, frames % 12, before_version, old_rgb);
        decode(new_group, frames % 12, 2, new_rgb);
        for (unsigned i = 0; i < PIXELS * 3; ++i) {
            raw[i] >>= 3;
            int a = (int)raw[i] - old_rgb[i], b = (int)raw[i] - new_rgb[i];
            errors[0] += a * a; errors[1] += b * b;
        }
        seams[0] += seam_error(old_rgb, raw); seams[1] += seam_error(new_rgb, raw);
        if (frames == 90) {
            picture("build/quality/reference.ppm", raw);
            picture("build/quality/before.ppm", old_rgb);
            picture("build/quality/after.ppm", new_rgb);
        }
        ++frames;
    }
    if (fgetc(reference) != EOF || ferror(reference)) { fputs("Reference frame count mismatch\n", stderr); return 1; }
    printf("{\"frames\":%u,\"before_psnr_db\":%.3f,\"after_psnr_db\":%.3f,\"squared_error_reduction_percent\":%.2f,\"boundary_error_reduction_percent\":%.2f}\n",
        frames, 10 * log10(961.0 * frames * PIXELS * 3 / errors[0]), 10 * log10(961.0 * frames * PIXELS * 3 / errors[1]),
        (1 - errors[1] / errors[0]) * 100, (1 - seams[1] / seams[0]) * 100);
    free(raw); free(old_rgb); free(new_rgb);
    fclose(before); fclose(after); fclose(reference);
    return errors[1] < errors[0] && seams[1] < seams[0] ? 0 : 1;
}
