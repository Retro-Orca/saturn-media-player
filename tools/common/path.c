/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "common.h"
#include <windows.h>

wchar_t *utf8_to_wide(const char *text)
{
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
    if (!count) fail("Invalid UTF-8 path");
    wchar_t *result = allocate((size_t)count * sizeof(*result));
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, result, count);
    return result;
}
