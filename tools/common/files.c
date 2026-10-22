/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <wchar.h>
#include <sys/stat.h>
#include <direct.h>

void fail(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    fputs("ERROR: ", stderr);
    vfprintf(stderr, format, arguments);
    fputc('\n', stderr);
    va_end(arguments);
    exit(1);
}

void *allocate(size_t size)
{
    void *result = calloc(size ? size : 1, 1);
    if (!result) fail("Out of memory (%zu bytes)", size);
    return result;
}

FILE *open_file(const char *path, const char *mode)
{
    wchar_t *name = utf8_to_wide(path);
    wchar_t *access = utf8_to_wide(mode);
    FILE *file = _wfopen(name, access);
    free(name);
    free(access);
    if (!file) fail("Cannot open %s", path);
    return file;
}

int file_exists(const char *path)
{
    wchar_t *name = utf8_to_wide(path);
    DWORD attributes = GetFileAttributesW(name);
    free(name);
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

uint64_t file_size(const char *path)
{
    wchar_t *name = utf8_to_wide(path);
    struct _stat64 info;
    int result = _wstat64(name, &info);
    free(name);
    if (result) fail("Cannot stat %s", path);
    return (uint64_t)info.st_size;
}

void make_directory(const char *path)
{
    wchar_t *name = utf8_to_wide(path);
    _wmkdir(name);
    free(name);
}

void write_all(FILE *file, const void *data, size_t size)
{
    if (fwrite(data, 1, size, file) != size) fail("File write failed");
}

void read_all(FILE *file, void *data, size_t size)
{
    if (fread(data, 1, size, file) != size) fail("Truncated file");
}
