/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>

static void both16(uint8_t *data, uint16_t value)
{
    data[0] = (uint8_t)value; data[1] = (uint8_t)(value >> 8); put_be16(data + 2, value);
}

static void both32(uint8_t *data, uint32_t value)
{
    put_le32(data, value); put_be32(data + 4, value);
}

static size_t directory_record(uint8_t *target, const uint8_t *name, size_t length, uint32_t lba, uint32_t bytes, int directory)
{
    size_t size = 33 + length + (length % 2 == 0);
    memset(target, 0, size);
    target[0] = (uint8_t)size;
    both32(target + 2, lba);
    both32(target + 10, bytes);
    target[18] = 126; target[19] = 10; target[20] = 22;
    target[25] = directory ? 2 : 0;
    both16(target + 28, 1);
    target[32] = (uint8_t)length;
    memcpy(target + 33, name, length);
    return size;
}

static void append_record(uint8_t *directory, size_t *position, const uint8_t *name, size_t length, uint32_t lba, uint32_t bytes, int is_directory)
{
    size_t size = 33 + length + (length % 2 == 0);
    if (*position % SECTOR_BYTES + size > SECTOR_BYTES) *position = (*position + SECTOR_BYTES - 1) / SECTOR_BYTES * SECTOR_BYTES;
    if (*position + size > 4 * SECTOR_BYTES) fail("ISO directory overflow");
    *position += directory_record(directory + *position, name, length, lba, bytes, is_directory);
}

static void copy_at(FILE *disc, const char *path, uint32_t lba)
{
    FILE *input = open_file(path, "rb");
    if (_fseeki64(disc, (int64_t)lba * SECTOR_BYTES, SEEK_SET)) fail("ISO seek failed");
    uint8_t buffer[65536];
    size_t size;
    while ((size = fread(buffer, 1, sizeof(buffer), input))) write_all(disc, buffer, size);
    if (ferror(input)) fail("ISO input read failed");
    fclose(input);
}

void create_disc(Movie *movies, int count)
{
    uint32_t root_bytes = (uint32_t)((110 + count * 44 + SECTOR_BYTES - 1) / SECTOR_BYTES * SECTOR_BYTES);
    uint32_t total = movies[count - 1].lba + 1 + movies[count - 1].groups * GROUP_SECTORS;
    if (total < 450) total = 450;
    FILE *disc = open_file("build/saturn-video.iso", "w+b");
    uint8_t zero[SECTOR_BYTES] = {0};
    for (uint32_t sector = 0; sector < total; ++sector) write_all(disc, zero, sizeof(zero));
    copy_at(disc, "build/IP.BIN", 0);
    uint8_t pvd[SECTOR_BYTES] = {0};
    memcpy(pvd, "\1CD001\1", 7);
    memset(pvd + 8, ' ', 64);
    memcpy(pvd + 8, "SEGA SATURN", 11);
    memcpy(pvd + 40, "SATURN_VIDEO_PLAYER", 19);
    both32(pvd + 80, total);
    both16(pvd + 120, 1); both16(pvd + 124, 1); both16(pvd + 128, SECTOR_BYTES);
    both32(pvd + 132, 10);
    put_le32(pvd + 140, 19); put_be32(pvd + 148, 20);
    const uint8_t dot = 0, parent = 1;
    directory_record(pvd + 156, &dot, 1, 21, root_bytes, 1);
    memset(pvd + 190, ' ', 623);
    memcpy(pvd + 813, "2026102200000000", 16);
    memcpy(pvd + 830, "2026102200000000", 16);
    memcpy(pvd + 847, "0000000000000000", 16);
    memcpy(pvd + 864, "0000000000000000", 16);
    pvd[881] = 1;
    _fseeki64(disc, 16 * SECTOR_BYTES, SEEK_SET); write_all(disc, pvd, sizeof(pvd));
    memset(pvd, 0, sizeof(pvd)); memcpy(pvd, "\377CD001\1", 7); write_all(disc, pvd, sizeof(pvd));
    uint8_t path[10] = {1, 0};
    put_le32(path + 2, 21); path[6] = 1;
    _fseeki64(disc, 19 * SECTOR_BYTES, SEEK_SET); write_all(disc, path, sizeof(path));
    put_be32(path + 2, 21); path[6] = 0; path[7] = 1;
    _fseeki64(disc, 20 * SECTOR_BYTES, SEEK_SET); write_all(disc, path, sizeof(path));
    uint8_t directory[4 * SECTOR_BYTES] = {0};
    size_t position = 0;
    append_record(directory, &position, &dot, 1, 21, root_bytes, 1);
    append_record(directory, &position, &parent, 1, 21, root_bytes, 1);
    append_record(directory, &position, (const uint8_t *)"1ST.BIN;1", 9, 32, (uint32_t)file_size("build/1ST.BIN"), 0);
    copy_at(disc, "build/1ST.BIN", 32);
    for (int i = 0; i < count; ++i) {
        char name[32]; snprintf(name, sizeof(name), "V%03d.SVT;1", i);
        append_record(directory, &position, (const uint8_t *)name, strlen(name), movies[i].lba, (uint32_t)file_size(movies[i].encoded), 0);
        copy_at(disc, movies[i].encoded, movies[i].lba);
    }
    _fseeki64(disc, 21 * SECTOR_BYTES, SEEK_SET); write_all(disc, directory, sizeof(directory));
    fclose(disc);
    FILE *cue = open_file("build/saturn-video.cue", "wb");
    fputs("FILE \"saturn-video.iso\" BINARY\n  TRACK 01 MODE1/2048\n    INDEX 01 00:00:00\n", cue);
    fclose(cue);
    FILE *report = open_file("build/report.json", "wb");
    fprintf(report, "{\n  \"runtime\": \"SH-2 assembly\",\n  \"build_tools\": \"C\",\n  \"width\": 320,\n  \"height\": 224,\n  \"nominal_fps\": 60,\n  \"stream_bytes_per_second\": 276480,\n  \"audio\": \"22050 Hz mono PCM16\",\n  \"movie_count\": %d,\n  \"iso_bytes\": %llu,\n  \"hardware_verified\": false\n}\n", count, (unsigned long long)total * SECTOR_BYTES);
    fclose(report);
}
