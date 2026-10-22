/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "codebook.h"
#include <stdlib.h>
#include <string.h>

TileSamples collect_samples(const uint8_t *tiles, unsigned count)
{
    TileSamples result = {allocate((size_t)count * TILE_COMPONENTS), allocate((size_t)count * sizeof(uint32_t)), 0};
    unsigned capacity = 1;
    while (capacity < count * 2) capacity *= 2;
    unsigned *table = allocate((size_t)capacity * sizeof(*table));
    /* 同じ小画像は出現回数を重みにしてまとめ、静止部分も正しく学習する */
    for (unsigned i = 0; i < count; ++i) {
        const uint8_t *tile = tiles + i * TILE_COMPONENTS;
        uint32_t hash = 2166136261u;
        for (unsigned c = 0; c < TILE_COMPONENTS; ++c) hash = (hash ^ tile[c]) * 16777619u;
        unsigned slot = hash & (capacity - 1);
        while (table[slot] && memcmp(result.pixels + (table[slot] - 1) * TILE_COMPONENTS, tile, TILE_COMPONENTS))
            slot = (slot + 1) & (capacity - 1);
        if (!table[slot]) {
            memcpy(result.pixels + result.count * TILE_COMPONENTS, tile, TILE_COMPONENTS);
            table[slot] = ++result.count;
        }
        ++result.weights[table[slot] - 1];
    }
    free(table);
    return result;
}
