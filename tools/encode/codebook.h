/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#ifndef SATURN_CODEBOOK_H
#define SATURN_CODEBOOK_H

#include "../common/common.h"

typedef struct {
    uint8_t *pixels;
    uint32_t *weights;
    unsigned count;
} TileSamples;

TileSamples collect_samples(const uint8_t *tiles, unsigned count);
void refine_codebook(const uint8_t *tiles, unsigned count, uint8_t *centers);

#endif
