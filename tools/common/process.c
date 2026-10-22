/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static PROCESS_INFORMATION start_process(const char *const *arguments, HANDLE output)
{
    size_t count = 0;
    while (arguments[count]) ++count;
    wchar_t **wide = allocate((count + 1) * sizeof(*wide));
    for (size_t i = 0; i < count; ++i) wide[i] = utf8_to_wide(arguments[i]);
    const char *program = NULL;
    if (!strcmp(arguments[0], "ffmpeg")) program = getenv("FFMPEG");
    if (!strcmp(arguments[0], "ffprobe")) program = getenv("FFPROBE");
    if (!program && !strcmp(arguments[0], "ffmpeg") && file_exists("vendor/media/ffmpeg.exe")) program = "vendor/media/ffmpeg.exe";
    if (!program && !strcmp(arguments[0], "ffprobe") && file_exists("vendor/media/ffprobe.exe")) program = "vendor/media/ffprobe.exe";
    if (program) { free(wide[0]); wide[0] = utf8_to_wide(program); }
    wchar_t command[32768];
    size_t used = 0;
    /* シェルを介さずWindowsの引数規則に合わせて引用符と末尾の逆斜線を処理する */
    for (size_t i = 0; i < count; ++i) {
        if (used + 4 >= 32768) fail("Command line is too long");
        if (i) command[used++] = L' ';
        command[used++] = L'"';
        size_t slashes = 0;
        for (const wchar_t *cursor = wide[i];; ++cursor) {
            if (*cursor == L'\\') { ++slashes; continue; }
            size_t copies = (*cursor == L'"' || !*cursor) ? slashes * 2 : slashes;
            if (used + copies + 4 >= 32768) fail("Command line is too long");
            while (copies--) command[used++] = L'\\';
            slashes = 0;
            if (!*cursor) break;
            if (*cursor == L'"') command[used++] = L'\\';
            command[used++] = *cursor;
        }
        command[used++] = L'"';
    }
    command[used] = 0;
    STARTUPINFOW startup = {0};
    PROCESS_INFORMATION child = {0};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = output ? output : GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    BOOL result = CreateProcessW(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startup, &child);
    for (size_t i = 0; i < count; ++i) free(wide[i]);
    free(wide);
    if (!result) fail("Cannot start %s (Windows error %lu)", arguments[0], GetLastError());
    CloseHandle(child.hThread);
    return child;
}

int run_process(const char *const *arguments)
{
    PROCESS_INFORMATION child = start_process(arguments, NULL);
    WaitForSingleObject(child.hProcess, INFINITE);
    DWORD result = 1;
    GetExitCodeProcess(child.hProcess, &result);
    CloseHandle(child.hProcess);
    return (int)result;
}

MediaPipe open_media_pipe(const char *const *arguments)
{
    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, TRUE};
    HANDLE reader = NULL, writer = NULL;
    if (!CreatePipe(&reader, &writer, &security, 1024 * 1024) || !SetHandleInformation(reader, HANDLE_FLAG_INHERIT, 0)) fail("Cannot create FFmpeg pipe");
    PROCESS_INFORMATION child = start_process(arguments, writer);
    CloseHandle(writer);
    MediaPipe pipe = {child.hProcess, reader};
    return pipe;
}

size_t read_media_pipe(MediaPipe *pipe, void *data, size_t size)
{
    size_t total = 0;
    while (total < size) {
        DWORD bytes = 0;
        DWORD request = (DWORD)((size - total) > 1048576 ? 1048576 : size - total);
        if (!ReadFile(pipe->output, (uint8_t *)data + total, request, &bytes, NULL)) {
            if (GetLastError() != ERROR_BROKEN_PIPE) fail("Cannot read FFmpeg output");
            break;
        }
        if (!bytes) break;
        total += bytes;
    }
    return total;
}

int close_media_pipe(MediaPipe *pipe, int cancel)
{
    if (!pipe->process) return 0;
    CloseHandle(pipe->output);
    DWORD result = 1;
    GetExitCodeProcess(pipe->process, &result);
    if (cancel && result == STILL_ACTIVE) {
        TerminateProcess(pipe->process, 0);
    }
    WaitForSingleObject(pipe->process, INFINITE);
    GetExitCodeProcess(pipe->process, &result);
    CloseHandle(pipe->process);
    pipe->process = NULL;
    pipe->output = NULL;
    return (int)result;
}
