/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "common.h"

void put_be16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)(value >> 8);
    data[1] = (uint8_t)value;
}

void put_be32(uint8_t *data, uint32_t value)
{
    for (int i = 3; i >= 0; --i) {
        data[i] = (uint8_t)value;
        value >>= 8;
    }
}

void put_le32(uint8_t *data, uint32_t value)
{
    for (int i = 0; i < 4; ++i) {
        data[i] = (uint8_t)value;
        value >>= 8;
    }
}

uint32_t get_be32(const uint8_t *data)
{
    return (uint32_t)data[0] << 24 | (uint32_t)data[1] << 16 |
           (uint32_t)data[2] << 8 | data[3];
}

uint32_t get_le32(const uint8_t *data)
{
    return (uint32_t)data[3] << 24 | (uint32_t)data[2] << 16 |
           (uint32_t)data[1] << 8 | data[0];
}
