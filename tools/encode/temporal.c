/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned error[GROUP_FRAMES];
    uint16_t mask[GROUP_FRAMES];
} UpdateChoices;

static UpdateChoices cell_choices(const uint8_t *tiles, const uint8_t *centers, const uint16_t *desired, unsigned cell)
{
    unsigned segment[GROUP_FRAMES][GROUP_FRAMES] = {{0}};
    unsigned cost[GROUP_FRAMES][GROUP_FRAMES];
    uint16_t masks[GROUP_FRAMES][GROUP_FRAMES] = {{0}};
    memset(cost, 0xFF, sizeof(cost));
    /* 更新した小画像を何フレーム保持するか、区間全体の誤差で判断する */
    for (unsigned start = 0; start < GROUP_FRAMES; ++start)
        for (unsigned end = start; end < GROUP_FRAMES; ++end) {
            unsigned error = end == start ? 0 : segment[start][end - 1];
            for (unsigned q = 0; q < 4; ++q) {
                unsigned source = (end * FRAME_PATCHES + cell) * 4 + q;
                unsigned index = desired[(start * FRAME_PATCHES + cell) * 4 + q];
                error += tile_distance(tiles + source * TILE_COMPONENTS, centers + index * TILE_COMPONENTS);
            }
            segment[start][end] = error;
        }
    for (unsigned end = 0; end < GROUP_FRAMES; ++end)
        for (unsigned extra = 0; extra <= end; ++extra)
            for (unsigned start = extra; start <= end; ++start) {
                if ((!extra && start) || (extra && !start)) continue;
                unsigned previous = start ? cost[start - 1][extra - 1] : 0;
                if (previous == UINT_MAX) continue;
                unsigned candidate = previous + segment[start][end];
                if (candidate < cost[end][extra]) {
                    cost[end][extra] = candidate;
                    masks[end][extra] = (uint16_t)((start ? masks[start - 1][extra - 1] : 0) | (1u << start));
                }
            }
    UpdateChoices result;
    memcpy(result.error, cost[GROUP_FRAMES - 1], sizeof(result.error));
    memcpy(result.mask, masks[GROUP_FRAMES - 1], sizeof(result.mask));
    return result;
}

void select_updates(const uint8_t *tiles, const uint8_t *centers, const uint16_t *desired, uint16_t *masks)
{
    const unsigned capacity = (SOUND_OFFSET - CODEBOOK_BYTES - GROUP_FRAMES * (4 + FRAME_BANK_BYTES)) / 5 - FRAME_PATCHES;
    UpdateChoices *choices = allocate(FRAME_PATCHES * sizeof(*choices));
    unsigned required = 0;
    for (unsigned cell = 0; cell < FRAME_PATCHES; ++cell) {
        choices[cell] = cell_choices(tiles, centers, desired, cell);
        unsigned best = 0;
        for (unsigned extra = 1; extra < GROUP_FRAMES; ++extra)
            if (choices[cell].error[extra] < choices[cell].error[best]) best = extra;
        required += best;
        masks[cell] = choices[cell].mask[best];
    }
    if (required > capacity) {
        /* 全セルで容量を共有するナップサックDPで、誤差の合計が最小の配分を選ぶ */
        const unsigned stride = capacity + 1;
        uint64_t *previous = allocate(stride * sizeof(*previous)), *next = allocate(stride * sizeof(*next));
        uint8_t *backtrack = allocate((size_t)FRAME_PATCHES * stride);
        for (unsigned bytes = 1; bytes <= capacity; ++bytes) previous[bytes] = UINT64_MAX;
        unsigned limit = 0;
        for (unsigned cell = 0; cell < FRAME_PATCHES; ++cell) {
            unsigned next_limit = limit + GROUP_FRAMES - 1;
            if (next_limit > capacity) next_limit = capacity;
            for (unsigned used = 0; used <= next_limit; ++used) {
                uint64_t best = UINT64_MAX;
                unsigned selected = 0;
                for (unsigned extra = 0; extra < GROUP_FRAMES && extra <= used; ++extra) {
                    if (used - extra > limit || previous[used - extra] == UINT64_MAX) continue;
                    uint64_t candidate = previous[used - extra] + choices[cell].error[extra];
                    if (candidate < best) { best = candidate; selected = extra; }
                }
                next[used] = best;
                backtrack[(size_t)cell * stride + used] = (uint8_t)selected;
            }
            uint64_t *temporary = previous; previous = next; next = temporary;
            limit = next_limit;
        }
        unsigned used = 0;
        for (unsigned i = 1; i <= capacity; ++i) if (previous[i] < previous[used]) used = i;
        for (unsigned cell = FRAME_PATCHES; cell-- > 0;) {
            unsigned extra = backtrack[(size_t)cell * stride + used];
            masks[cell] = choices[cell].mask[extra];
            used -= extra;
        }
        free(backtrack); free(next); free(previous);
    }
    free(choices);
}
