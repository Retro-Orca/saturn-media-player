/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <string.h>

void verify_disc(const char *path)
{
    FILE *file = open_file(path, "rb");
    uint64_t size = file_size(path);
    uint8_t sector[SECTOR_BYTES];
    read_all(file, sector, sizeof(sector));
    if (memcmp(sector, "SEGA SEGASATURN ", 16) || get_be32(sector + 0xF0) != 0x06004000 || get_be32(sector + 0xE0) != 4096) fail("Invalid Saturn boot header");
    _fseeki64(file, 16 * SECTOR_BYTES, SEEK_SET); read_all(file, sector, sizeof(sector));
    if (memcmp(sector, "\1CD001\1", 7) || (uint64_t)get_le32(sector + 80) * SECTOR_BYTES != size || size % SECTOR_BYTES) fail("Invalid ISO volume size");
    uint32_t root = get_le32(sector + 158);
    uint32_t root_size = get_le32(sector + 166);
    int files = 0, movies = 0;
    for (uint32_t offset = 0; offset < root_size; offset += SECTOR_BYTES) {
        _fseeki64(file, (int64_t)root * SECTOR_BYTES + offset, SEEK_SET); read_all(file, sector, sizeof(sector));
        for (unsigned position = 0; position < SECTOR_BYTES && sector[position];) {
            const uint8_t *record = sector + position;
            unsigned length = record[0];
            if (length < 34 || position + length > SECTOR_BYTES || 33u + record[32] > length) fail("Invalid ISO directory");
            if (!(record[25] & 2)) {
                uint32_t lba = get_le32(record + 2), bytes = get_le32(record + 10);
                if ((uint64_t)lba * SECTOR_BYTES + bytes > size) fail("File extent outside ISO");
                if (!files && (record[32] != 9 || memcmp(record + 33, "1ST.BIN;1", 9))) fail("First boot file is not 1ST.BIN");
                if (files) {
                    uint8_t header[SECTOR_BYTES];
                    _fseeki64(file, (int64_t)lba * SECTOR_BYTES, SEEK_SET); read_all(file, header, sizeof(header));
                    uint32_t groups = get_be32(header + 20), frames = get_be32(header + 16);
                    if (memcmp(header, "SVT2", 4) || get_be32(header + 4) != 2 || get_be32(header + 40) != CODEBOOK_ENTRIES ||
                        get_be32(header + 44) != SOUND_OFFSET || !frames || groups != (frames + 11) / 12 ||
                        bytes != SECTOR_BYTES + (uint64_t)groups * GROUP_BYTES) fail("Invalid video extent");
                    ++movies;
                }
                ++files;
            }
            position += length;
        }
    }
    fclose(file);
    if (!movies) fail("No videos in ISO");
    printf("Verified ISO: %d videos, %llu bytes\n", movies, (unsigned long long)size);
}
