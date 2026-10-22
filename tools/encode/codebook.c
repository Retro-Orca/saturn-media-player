/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "codebook.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <emmintrin.h>

unsigned tile_distance(const uint8_t *left, const uint8_t *right)
{
    __m128i total = _mm_setzero_si128(), zero = _mm_setzero_si128();
    for (int i = 0; i < TILE_COMPONENTS; i += 16) {
        __m128i a = _mm_loadu_si128((const __m128i *)(left + i));
        __m128i b = _mm_loadu_si128((const __m128i *)(right + i));
        __m128i low = _mm_sub_epi16(_mm_unpacklo_epi8(a, zero), _mm_unpacklo_epi8(b, zero));
        __m128i high = _mm_sub_epi16(_mm_unpackhi_epi8(a, zero), _mm_unpackhi_epi8(b, zero));
        total = _mm_add_epi32(total, _mm_add_epi32(_mm_madd_epi16(low, low), _mm_madd_epi16(high, high)));
    }
    uint32_t sums[4];
    _mm_storeu_si128((__m128i *)sums, total);
    return sums[0] + sums[1] + sums[2] + sums[3];
}

static unsigned nearest(const uint8_t *tile, const uint8_t *centers, unsigned first, unsigned count, unsigned *error)
{
    unsigned best = UINT_MAX, index = first;
    for (unsigned i = first; i < first + count; ++i) {
        unsigned value = tile_distance(tile, centers + i * TILE_COMPONENTS);
        if (value < best) { best = value; index = i; }
        if (!value) break;
    }
    *error = best;
    return index;
}

unsigned nearest_codeword(const uint8_t *tile, const uint8_t *centers)
{
    unsigned error;
    return nearest(tile, centers, 0, CODEBOOK_ENTRIES, &error);
}

void train_codebook(const uint8_t *tiles, int count, uint8_t *centers)
{
    const int seeds = count < 4096 ? count : 4096;
    const int samples = count < 8192 ? count : 8192;
    uint32_t *sums = allocate(CODEBOOK_ENTRIES * TILE_COMPONENTS * sizeof(*sums));
    unsigned *errors = allocate((size_t)seeds * sizeof(*errors));
    uint32_t weights[CODEBOOK_ENTRIES];
    for (int i = 0; i < seeds; ++i) errors[i] = UINT_MAX;
    int selected = 0;
    /* 最も誤差の大きい小画像を順に選び、細い輪郭も辞書の初期値に含める */
    for (int entry = 0; entry < CODEBOOK_ENTRIES; ++entry) {
        int source = (int)((uint64_t)selected * (unsigned)count / (unsigned)seeds);
        memcpy(centers + entry * TILE_COMPONENTS, tiles + source * TILE_COMPONENTS, TILE_COMPONENTS);
        unsigned largest = 0;
        for (int i = 0; i < seeds; ++i) {
            source = (int)((uint64_t)i * (unsigned)count / (unsigned)seeds);
            unsigned value = tile_distance(tiles + source * TILE_COMPONENTS, centers + entry * TILE_COMPONENTS);
            if (value < errors[i]) errors[i] = value;
            if (errors[i] > largest) { largest = errors[i]; selected = i; }
        }
    }
    for (int iteration = 0; iteration < 12; ++iteration) {
        memset(sums, 0, CODEBOOK_ENTRIES * TILE_COMPONENTS * sizeof(*sums));
        memset(weights, 0, sizeof(weights));
        unsigned largest = 0, worst = 0;
        for (int i = 0; i < samples; ++i) {
            unsigned source = (unsigned)((uint64_t)i * (unsigned)count / (unsigned)samples);
            const uint8_t *tile = tiles + source * TILE_COMPONENTS;
            unsigned error;
            unsigned index = nearest(tile, centers, 0, CODEBOOK_ENTRIES, &error);
            if (error > largest) { largest = error; worst = source; }
            ++weights[index];
            for (int c = 0; c < TILE_COMPONENTS; ++c) sums[index * TILE_COMPONENTS + c] += tile[c];
        }
        for (unsigned entry = 0; entry < CODEBOOK_ENTRIES; ++entry) {
            if (weights[entry]) {
                for (int c = 0; c < TILE_COMPONENTS; ++c)
                    centers[entry * TILE_COMPONENTS + c] = (uint8_t)((sums[entry * TILE_COMPONENTS + c] + weights[entry] / 2) / weights[entry]);
            } else {
                unsigned source = (worst + entry * 137) % (unsigned)count;
                memcpy(centers + entry * TILE_COMPONENTS, tiles + source * TILE_COMPONENTS, TILE_COMPONENTS);
            }
        }
    }
    free(errors);
    free(sums);
    refine_codebook(tiles, (unsigned)count, centers);
}
