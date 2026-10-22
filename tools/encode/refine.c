/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "codebook.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TileSamples samples;
    uint16_t *assignment;
    unsigned *errors;
    uint64_t distortion[CODEBOOK_ENTRIES];
    uint64_t removal[CODEBOOK_ENTRIES];
} Training;

/* 三角不等式で結果が変わらない候補だけを省き、全小画像を調べる */
static uint64_t classify(Training *training, const uint8_t *centers, int second_nearest)
{
    unsigned separation[CODEBOOK_ENTRIES][CODEBOOK_ENTRIES];
    for (unsigned a = 0; a < CODEBOOK_ENTRIES; ++a)
        for (unsigned b = 0; b <= a; ++b)
            separation[a][b] = separation[b][a] = tile_distance(centers + a * TILE_COMPONENTS, centers + b * TILE_COMPONENTS);
    memset(training->distortion, 0, sizeof(training->distortion));
    memset(training->removal, 0, sizeof(training->removal));
    uint64_t total = 0;
    for (unsigned i = 0; i < training->samples.count; ++i) {
        const uint8_t *tile = training->samples.pixels + i * TILE_COMPONENTS;
        unsigned anchor = training->assignment[i], index = anchor;
        unsigned best = tile_distance(tile, centers + anchor * TILE_COMPONENTS), next = UINT_MAX;
        unsigned bound = best * 4;
        for (unsigned entry = 0; entry < CODEBOOK_ENTRIES; ++entry) {
            if (entry == anchor || (!second_nearest && separation[anchor][entry] >= bound)) continue;
            unsigned error = tile_distance(tile, centers + entry * TILE_COMPONENTS);
            if (error < best) { next = best; best = error; index = entry; }
            else if (error < next) next = error;
        }
        training->assignment[i] = (uint16_t)index;
        training->errors[i] = best;
        uint64_t weight = training->samples.weights[i];
        total += weight * best;
        training->distortion[index] += weight * best;
        if (second_nearest) training->removal[index] += weight * (next - best);
    }
    return total;
}

static uint64_t improve(Training *training, uint8_t *centers, unsigned iterations)
{
    uint32_t sums[CODEBOOK_ENTRIES][TILE_COMPONENTS], weights[CODEBOOK_ENTRIES];
    for (unsigned iteration = 0; iteration < iterations; ++iteration) {
        classify(training, centers, 0);
        memset(sums, 0, sizeof(sums)); memset(weights, 0, sizeof(weights));
        for (unsigned i = 0; i < training->samples.count; ++i) {
            unsigned index = training->assignment[i], weight = training->samples.weights[i];
            const uint8_t *tile = training->samples.pixels + i * TILE_COMPONENTS;
            weights[index] += weight;
            for (unsigned c = 0; c < TILE_COMPONENTS; ++c) sums[index][c] += tile[c] * weight;
        }
        int changed = 0;
        for (unsigned entry = 0; entry < CODEBOOK_ENTRIES; ++entry) {
            if (!weights[entry]) continue;
            for (unsigned c = 0; c < TILE_COMPONENTS; ++c) {
                uint8_t mean = (uint8_t)((sums[entry][c] + weights[entry] / 2) / weights[entry]);
                changed |= centers[entry * TILE_COMPONENTS + c] != mean;
                centers[entry * TILE_COMPONENTS + c] = mean;
            }
        }
        if (!changed) break;
    }
    return classify(training, centers, 0);
}

void refine_codebook(const uint8_t *tiles, unsigned count, uint8_t *centers)
{
    Training training = {0};
    training.samples = collect_samples(tiles, count);
    training.assignment = allocate(training.samples.count * sizeof(*training.assignment));
    training.errors = allocate(training.samples.count * sizeof(*training.errors));
    uint64_t best = improve(&training, centers, 24);
    uint8_t saved[CODEBOOK_ENTRIES * TILE_COMPONENTS];
    /* 役立っていない辞書を誤差の大きい場所へ移し、悪化する試行は採用しない */
    for (unsigned trial = 0; trial < 8 && best; ++trial) {
        classify(&training, centers, 1);
        unsigned remove = 0, split = 0;
        for (unsigned entry = 1; entry < CODEBOOK_ENTRIES; ++entry) {
            if (training.removal[entry] < training.removal[remove]) remove = entry;
            if (training.distortion[entry] > training.distortion[split]) split = entry;
        }
        if (remove == split) break;
        unsigned source = 0; uint64_t largest = 0;
        for (unsigned i = 0; i < training.samples.count; ++i) {
            uint64_t error = (uint64_t)training.errors[i] * training.samples.weights[i];
            if (training.assignment[i] == split && error > largest) { largest = error; source = i; }
        }
        if (largest <= training.removal[remove]) break;
        memcpy(saved, centers, sizeof(saved));
        memcpy(centers + remove * TILE_COMPONENTS, training.samples.pixels + source * TILE_COMPONENTS, TILE_COMPONENTS);
        uint64_t candidate = improve(&training, centers, 6);
        if (candidate >= best) { memcpy(centers, saved, sizeof(saved)); break; }
        best = candidate;
    }
    free(training.assignment); free(training.errors);
    free(training.samples.pixels); free(training.samples.weights);
}
