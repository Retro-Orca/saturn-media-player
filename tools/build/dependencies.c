/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>

void validate_build_dependencies(void)
{
    const char *assembler = getenv("SATURN_AS");
    if (!assembler || !*assembler) assembler = "vendor/assembler/asw.exe";
    if (!file_exists(assembler))
        fail("Macro Assembler AS is missing; set SATURN_AS to asw.exe or see docs/setup.md");
    if (!file_exists("vendor/boot/saturn_common.bin"))
        fail("Local Saturn boot area is missing; run build.cmd --import-boot PATH_TO_IP.BIN (docs/setup.md)");
    if (file_size("vendor/boot/saturn_common.bin") != 0xD80)
        fail("Invalid local Saturn boot area; expected 3456 bytes");
}
