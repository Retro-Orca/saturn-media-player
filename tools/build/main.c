/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static void make_demo(void)
{
    make_directory("build/demo");
    const char *arguments[] = {"ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
        "-f", "lavfi", "-i", "testsrc2=size=320x224:rate=60:duration=6",
        "-f", "lavfi", "-i", "sine=frequency=440:sample_rate=22050:duration=6",
        "-c:v", "libx264", "-pix_fmt", "yuv420p", "-crf", "12", "-c:a", "aac", "-shortest", "build/demo/60fps-test.mp4", NULL};
    if (run_process(arguments)) fail("Cannot generate demo");
}

static int main_utf8(int argc, char **argv)
{
    SetConsoleOutputCP(CP_UTF8);
    make_directory("build"); make_directory("build/media");
    if (argc > 1 && !strcmp(argv[1], "--test")) return run_self_tests();
    if (argc > 1 && !strcmp(argv[1], "--verify")) { verify_disc(argc > 2 ? argv[2] : "build/saturn-video.iso"); return 0; }
    if (argc > 1 && !strcmp(argv[1], "--import-boot")) {
        if (argc != 3) fail("Usage: saturn-build.exe --import-boot PATH_TO_IP.BIN");
        import_bootstrap(argv[2]);
        return 0;
    }
    int demo = 0, force = 0;
    const char *directory = "videos";
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--demo")) demo = 1;
        else if (!strcmp(argv[i], "--reencode")) force = 1;
        else if (!strcmp(argv[i], "--input") && i + 1 < argc) directory = argv[++i];
        else fail("Unknown option: %s", argv[i]);
    }
    validate_build_dependencies();
    Movie *movies = allocate(MAX_MOVIES * sizeof(*movies));
    int count;
    if (demo) { make_demo(); count = find_movies(movies, "build/demo"); }
    else count = find_movies(movies, directory);
    if (!count) fail("Put videos in the videos folder, or run build.cmd --demo");
    for (int i = 0; i < count; ++i) encode_movie(&movies[i], force);
    make_font_and_titles(movies, count);
    write_catalog(movies, count);
    assemble_program();
    create_disc(movies, count);
    verify_disc("build/saturn-video.iso");
    free(movies);
    puts("Created build/saturn-video.cue and build/saturn-video.iso");
    return 0;
}

int wmain(int argc, wchar_t **wide_arguments)
{
    char **arguments = allocate((size_t)(argc + 1) * sizeof(*arguments));
    for (int i = 0; i < argc; ++i) {
        int bytes = WideCharToMultiByte(CP_UTF8, 0, wide_arguments[i], -1, NULL, 0, NULL, NULL);
        arguments[i] = allocate((size_t)bytes);
        WideCharToMultiByte(CP_UTF8, 0, wide_arguments[i], -1, arguments[i], bytes, NULL, NULL);
    }
    int result = main_utf8(argc, arguments);
    for (int i = 0; i < argc; ++i) free(arguments[i]);
    free(arguments);
    return result;
}
