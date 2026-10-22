/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <stdlib.h>
#include <string.h>

void write_video_header(FILE *file, uint32_t frames, uint32_t groups)
{
    uint8_t header[SECTOR_BYTES] = {0};
    memcpy(header, "SVT1", 4);
    put_be32(header + 4, 1);
    put_be32(header + 8, VIDEO_WIDTH);
    put_be32(header + 12, VIDEO_HEIGHT);
    put_be32(header + 16, frames);
    put_be32(header + 20, groups);
    put_be32(header + 24, GROUP_FRAMES);
    put_be32(header + 28, GROUP_SECTORS);
    put_be32(header + 32, 22050);
    put_be32(header + 36, AUDIO_SAMPLES);
    write_all(file, header, sizeof(header));
}

void read_video_header(Movie *movie)
{
    uint8_t header[SECTOR_BYTES];
    FILE *file = open_file(movie->encoded, "rb");
    read_all(file, header, sizeof(header));
    fclose(file);
    if (memcmp(header, "SVT1", 4) || get_be32(header + 4) != 1 ||
        get_be32(header + 8) != VIDEO_WIDTH || get_be32(header + 12) != VIDEO_HEIGHT ||
        get_be32(header + 24) != GROUP_FRAMES || get_be32(header + 28) != GROUP_SECTORS ||
        get_be32(header + 32) != 22050 || get_be32(header + 36) != AUDIO_SAMPLES) fail("Invalid SVT header: %s", movie->encoded);
    movie->frames = get_be32(header + 16);
    movie->groups = get_be32(header + 20);
    if (!movie->frames || !movie->groups || movie->groups != (movie->frames + GROUP_FRAMES - 1) / GROUP_FRAMES ||
        file_size(movie->encoded) != SECTOR_BYTES + (uint64_t)movie->groups * GROUP_BYTES) fail("Invalid SVT length: %s", movie->encoded);
}

void encode_movie(Movie *movie, int force)
{
    char cache_path[PATH_CAPACITY + 16];
    snprintf(cache_path, sizeof(cache_path), "%s.cache", movie->encoded);
    /* 入力全体のハッシュを取りファイル名の変更と同サイズの差し替えも検出する */
    FILE *source = open_file(movie->source, "rb");
    uint64_t hash = UINT64_C(14695981039346656037);
    uint8_t buffer[65536];
    size_t length;
    while ((length = fread(buffer, 1, sizeof(buffer), source)))
        for (size_t i = 0; i < length; ++i) { hash ^= buffer[i]; hash *= UINT64_C(1099511628211); }
    if (ferror(source)) fail("Cannot read %s", movie->source);
    fclose(source);
    if (!force && file_exists(cache_path) && file_exists(movie->encoded)) {
        FILE *cache = open_file(cache_path, "rb");
        unsigned long long previous = 0;
        int version = 0;
        int fields = fscanf(cache, "%d %llx", &version, &previous);
        fclose(cache);
        if (fields == 2 && version == 1 && previous == hash) {
            read_video_header(movie);
            printf("Cached: %s\n", movie->source);
            return;
        }
    }
    const char *video_args[] = {"ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", movie->source,
        "-map", "0:v:0", "-an", "-vf", "fps=60,scale=320:224:force_original_aspect_ratio=decrease:flags=lanczos,pad=320:224:(ow-iw)/2:(oh-ih)/2:black,setsar=1",
        "-pix_fmt", "rgb24", "-f", "rawvideo", "pipe:1", NULL};
    const char *audio_args[] = {"ffmpeg", "-hide_banner", "-loglevel", "error", "-y", "-i", movie->source,
        "-map", "0:a:0", "-vn", "-ac", "1", "-ar", "22050", "-f", "s16be", "pipe:1", NULL};
    const char *probe_args[] = {"ffprobe", "-v", "error", "-select_streams", "a:0", "-show_entries", "stream=index", "-of", "csv=p=0", "-o", "build/media/audio-stream.txt", movie->source, NULL};
    if (run_process(probe_args)) fail("Cannot inspect input audio: %s", movie->source);
    int has_audio = file_size("build/media/audio-stream.txt") != 0;
    const size_t frame_size = VIDEO_WIDTH * VIDEO_HEIGHT * 3;
    MediaPipe video = open_media_pipe(video_args);
    MediaPipe audio = {0};
    if (has_audio) audio = open_media_pipe(audio_args);
    FILE *output = open_file(movie->encoded, "wb");
    write_video_header(output, 0, 0);
    movie->frames = 0;
    movie->groups = 0;
    uint8_t *frames = allocate(frame_size * GROUP_FRAMES);
    uint8_t sound[AUDIO_BYTES];
    for (;;) {
        size_t bytes = read_media_pipe(&video, frames, frame_size * GROUP_FRAMES);
        if (!bytes) break;
        if (bytes % frame_size || movie->frames > UINT32_MAX - GROUP_FRAMES) fail("Invalid converted video length");
        uint32_t current = (uint32_t)(bytes / frame_size);
        for (uint32_t frame = current; frame < GROUP_FRAMES; ++frame)
            memcpy(frames + frame * frame_size, frames + (current - 1) * frame_size, frame_size);
        memset(sound, 0, sizeof(sound));
        if (has_audio) (void)read_media_pipe(&audio, sound, sizeof(sound));
        encode_group(output, frames, sound);
        movie->frames += current;
        ++movie->groups;
        if (movie->groups % 5 == 0) {
            printf("Encoding %s: %.1f seconds\n", movie->title, movie->frames / 60.0);
            fflush(stdout);
        }
    }
    free(frames);
    if (close_media_pipe(&video, 0)) fail("FFmpeg video conversion failed: %s", movie->source);
    if (has_audio && close_media_pipe(&audio, 1)) fail("FFmpeg audio conversion failed: %s", movie->source);
    if (!movie->frames) fail("The input video contains no frames");
    if (fseek(output, 0, SEEK_SET)) fail("Cannot finalize video header");
    write_video_header(output, movie->frames, movie->groups);
    fclose(output);
    FILE *cache = open_file(cache_path, "wb");
    fprintf(cache, "1 %016llx\n", (unsigned long long)hash);
    fclose(cache);
    read_video_header(movie);
}
