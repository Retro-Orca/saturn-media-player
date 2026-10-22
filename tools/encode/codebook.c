/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <emmintrin.h>

static unsigned distance(const uint8_t *left, const uint8_t *right)
{
    __m128i total = _mm_setzero_si128();
    for (int i = 0; i < TILE_COMPONENTS; i += 16) {
        __m128i a = _mm_loadu_si128((const __m128i *)(left + i));
        __m128i b = _mm_loadu_si128((const __m128i *)(right + i));
        total = _mm_add_epi64(total, _mm_sad_epu8(a, b));
    }
    uint64_t halves[2];
    _mm_storeu_si128((__m128i *)halves, total);
    return (unsigned)(halves[0] + halves[1]);
}

unsigned nearest_codeword(const uint8_t *tile, const uint8_t *centers)
{
    unsigned best = UINT_MAX;
    unsigned index = 0;
    for (unsigned i = 0; i < CODEBOOK_ENTRIES; ++i) {
        unsigned error = distance(tile, centers + i * TILE_COMPONENTS);
        if (error < best) { best = error; index = i; }
        if (!error) break;
    }
    return index;
}

void train_codebook(const uint8_t *tiles, int count, uint8_t *centers)
{
    const int samples = count < 3072 ? count : 3072;
    uint32_t *sums = allocate(CODEBOOK_ENTRIES * TILE_COMPONENTS * sizeof(*sums));
    uint32_t weights[CODEBOOK_ENTRIES];
    for (int i = 0; i < CODEBOOK_ENTRIES; ++i) {
        int source = (int)(((uint64_t)i * (unsigned)count) / CODEBOOK_ENTRIES);
        memcpy(centers + i * TILE_COMPONENTS, tiles + source * TILE_COMPONENTS, TILE_COMPONENTS);
    }
    /* 同じ小画像が続いても結果が変わらないように初期値と走査順を固定する */
    for (int iteration = 0; iteration < 7; ++iteration) {
        memset(sums, 0, CODEBOOK_ENTRIES * TILE_COMPONENTS * sizeof(*sums));
        memset(weights, 0, sizeof(weights));
        for (int i = 0; i < samples; ++i) {
            int source = (int)(((uint64_t)i * (unsigned)count) / (unsigned)samples);
            const uint8_t *tile = tiles + source * TILE_COMPONENTS;
            unsigned nearest = nearest_codeword(tile, centers);
            ++weights[nearest];
            for (int component = 0; component < TILE_COMPONENTS; ++component)
                sums[nearest * TILE_COMPONENTS + component] += tile[component];
        }
        for (int i = 0; i < CODEBOOK_ENTRIES; ++i) {
            if (weights[i]) {
                for (int component = 0; component < TILE_COMPONENTS; ++component)
                    centers[i * TILE_COMPONENTS + component] = (uint8_t)((sums[i * TILE_COMPONENTS + component] + weights[i] / 2) / weights[i]);
            } else {
                int source = (i * 127 + iteration * 313) % count;
                memcpy(centers + i * TILE_COMPONENTS, tiles + source * TILE_COMPONENTS, TILE_COMPONENTS);
            }
        }
    }
    free(sums);
}
