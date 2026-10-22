/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static int compare_movies(const void *left, const void *right)
{
    return strcmp(((const Movie *)left)->source, ((const Movie *)right)->source);
}

int find_movies(Movie *movies, const char *directory)
{
    char pattern[PATH_CAPACITY];
    snprintf(pattern, sizeof(pattern), "%s/*", directory);
    wchar_t *wide = utf8_to_wide(pattern);
    WIN32_FIND_DATAW data;
    HANDLE search = FindFirstFileW(wide, &data);
    free(wide);
    int count = 0;
    if (search == INVALID_HANDLE_VALUE) return 0;
    do {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        const wchar_t *extension = wcsrchr(data.cFileName, L'.');
        if (!extension || (_wcsicmp(extension, L".mp4") && _wcsicmp(extension, L".mkv") &&
            _wcsicmp(extension, L".mov") && _wcsicmp(extension, L".avi") &&
            _wcsicmp(extension, L".webm") && _wcsicmp(extension, L".m4v") &&
            _wcsicmp(extension, L".mpeg") && _wcsicmp(extension, L".mpg"))) continue;
        if (count == MAX_MOVIES) fail("At most %d videos are supported", MAX_MOVIES);
        char name[1024];
        if (!WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, name, sizeof(name), NULL, NULL)) fail("Video filename is too long");
        Movie *movie = &movies[count++];
        if (snprintf(movie->source, sizeof(movie->source), "%s/%s", directory, name) >= (int)sizeof(movie->source)) fail("Video path is too long");
        wchar_t title[256];
        size_t title_length = (size_t)(extension - data.cFileName);
        if (title_length > 80) title_length = 80;
        memcpy(title, data.cFileName, title_length * sizeof(*title));
        title[title_length] = 0;
        WideCharToMultiByte(CP_UTF8, 0, title, -1, movie->title, sizeof(movie->title), NULL, NULL);
    } while (FindNextFileW(search, &data));
    FindClose(search);
    qsort(movies, (size_t)count, sizeof(*movies), compare_movies);
    for (int i = 0; i < count; ++i) snprintf(movies[i].encoded, sizeof(movies[i].encoded), "build/media/%03d.svt", i);
    return count;
}
