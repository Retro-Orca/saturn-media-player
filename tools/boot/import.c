/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <string.h>

void import_bootstrap(const char *path)
{
    uint8_t header[0xE80];
    if (file_size(path) < sizeof(header)) fail("IP.BIN is shorter than the required boot area");
    FILE *input = open_file(path, "rb");
    read_all(input, header, sizeof(header));
    fclose(input);
    if (memcmp(header, "SEGA SEGASATURN ", 16)) fail("The supplied file is not a Saturn IP.BIN");

    /* この起動入口は4地域の領域を前提にするため配置が異なるIPは受け付けない */
    static const char *areas[] = {"For JAPAN.", "For TAIWAN and PHILIPINES.", "For USA and CANADA.", "For EUROPE."};
    for (unsigned i = 0; i < 4; ++i)
        if (memcmp(header + 0xE04 + i * 32, areas[i], strlen(areas[i])))
            fail("Unsupported IP.BIN area layout; see docs/setup.md");

    make_directory("vendor");
    make_directory("vendor/boot");
    FILE *output = open_file("vendor/boot/saturn_common.bin", "wb");
    write_all(output, header + 0x100, 0xD80);
    fclose(output);
    puts("Imported vendor/boot/saturn_common.bin for local builds (excluded from Git)");
}
