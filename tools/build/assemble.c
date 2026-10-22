/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>

static void object_to_binary(const char *input, const char *output, uint32_t base, uint32_t limit)
{
    FILE *file = open_file(input, "rb");
    size_t size = (size_t)file_size(input);
    uint8_t *data = allocate(size);
    read_all(file, data, size);
    fclose(file);
    if (size < 3 || data[0] != 0x89 || data[1] != 0x14) fail("Invalid assembler object");
    uint8_t *binary = allocate(limit - base);
    uint8_t *used = allocate(limit - base);
    uint32_t end = 0;
    size_t position = 2;
    while (position < size) {
        unsigned tag = data[position++];
        if (!tag) break;
        if (tag == 0x80) { position += 4; continue; }
        if (tag == 0x81) {
            if (position + 3 > size || data[position + 1] != 1 || data[position + 2] != 1) fail("Unsupported assembler segment");
            position += 3;
        } else if (tag >= 0x80) fail("Unknown assembler record");
        if (position + 6 > size) fail("Truncated assembler record");
        uint32_t address = get_le32(data + position);
        unsigned length = data[position + 4] | data[position + 5] << 8;
        position += 6;
        if (address < base || address > limit || length > limit - address || position + length > size) fail("Assembler output exceeds memory map");
        uint32_t offset = address - base;
        for (unsigned i = 0; i < length; ++i) {
            if (used[offset + i]) fail("Overlapping assembler sections");
            used[offset + i] = 1;
            binary[offset + i] = data[position + i];
        }
        if (offset + length > end) end = offset + length;
        position += length;
    }
    if (!end) fail("Empty executable");
    file = open_file(output, "wb");
    write_all(file, binary, end);
    fclose(file);
    free(data);
    free(binary);
    free(used);
}

void write_catalog(Movie *movies, int count)
{
    /* 実行ファイル用に領域を固定し動画の配置がコードサイズに依存しないようにする */
    uint32_t lba = 256;
    FILE *file = open_file("build/catalog.inc", "wb");
    fprintf(file, "MOVIE_COUNT equ %d\n    align 4\nmovie_catalog:\n", count);
    for (int i = 0; i < count; ++i) {
        movies[i].lba = lba;
        fprintf(file, "    dc.l $%08X,%u,%u,title_%03d\n", lba + 1, movies[i].groups, movies[i].frames, i);
        lba += 1 + movies[i].groups * GROUP_SECTORS;
    }
    for (int i = 0; i < count; ++i)
        fprintf(file, "title_%03d:\n    binclude \"build/title-%03d.bin\"\n", i, i);
    fclose(file);
    if (lba > 333000) fail("Videos exceed the conservative 650 MB disc capacity");
}

static void assemble(const char *source, const char *object, const char *listing)
{
    const char *assembler = getenv("SATURN_AS");
    if (!assembler || !*assembler) assembler = "vendor/assembler/asw.exe";
    if (getenv("SATURN_TEST")) {
        const char *test_arguments[] = {assembler, source, "-i", "src;.", "-bigendian", "-D", "TEST_BUILD", "-L", "-olist", listing,
            "-o", object, "-gnuerrors", "-maxerrors", "40", NULL};
        if (run_process(test_arguments)) fail("Test assembly failed: %s", source);
        return;
    }
    const char *arguments[] = {assembler, source, "-i", "src;.", "-bigendian", "-L", "-olist", listing,
        "-o", object, "-gnuerrors", "-maxerrors", "40", NULL};
    if (run_process(arguments)) fail("Assembly failed: %s", source);
}

void assemble_program(void)
{
    assemble("src/main.asm", "build/player.p", "build/player.lst");
    object_to_binary("build/player.p", "build/1ST.BIN", 0x06004000, 0x06020000);
    if (file_size("build/1ST.BIN") > (256 - 32) * SECTOR_BYTES) fail("Boot executable exceeds its disc reservation");
    assemble("src/boot/entry.asm", "build/boot.p", "build/boot.lst");
    object_to_binary("build/boot.p", "build/aip.bin", 0x06002E80, 0x06003000);
    uint8_t ip[4096] = {0};
    memset(ip, ' ', 0xD0);
    memcpy(ip, "SEGA SEGASATURN ", 16);
    memcpy(ip + 16, "SEGA ENTERPRISES", 16);
    memcpy(ip + 0x20, "SATV-00001V1.00020261022CD-1/1  ", 32);
    memcpy(ip + 0x40, "JTUE", 4);
    memcpy(ip + 0x50, "J", 1);
    memcpy(ip + 0x60, "SATURN VIDEO PLAYER", 19);
    put_be32(ip + 0xE0, sizeof(ip));
    put_be32(ip + 0xE8, 0x060FF000);
    put_be32(ip + 0xEC, 0x060FE000);
    put_be32(ip + 0xF0, 0x06004000);
    put_be32(ip + 0xF4, (uint32_t)file_size("build/1ST.BIN"));
    if (file_size("vendor/boot/saturn_common.bin") != 0xD80) fail("Invalid Saturn bootstrap size");
    FILE *file = open_file("vendor/boot/saturn_common.bin", "rb");
    read_all(file, ip + 256, 0xD80);
    fclose(file);
    file = open_file("build/aip.bin", "rb");
    read_all(file, ip + 0xE80, (size_t)file_size("build/aip.bin"));
    fclose(file);
    file = open_file("build/IP.BIN", "wb");
    write_all(file, ip, sizeof(ip));
    fclose(file);
}
