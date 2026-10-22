/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#ifndef SATURN_COMMON_H
#define SATURN_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 224
#define TILE_SIDE 4
#define TILE_BYTES 32
#define TILE_COMPONENTS 48
#define CODEBOOK_ENTRIES 384
#define CODEBOOK_BYTES 12288
#define FRAME_PATCHES 1120
#define FRAME_BANK_BYTES 140
#define SOUND_OFFSET 46476
#define GROUP_FRAMES 12
#define AUDIO_SAMPLES 4410
#define AUDIO_BYTES 8820
#define SECTOR_BYTES 2048
#define GROUP_SECTORS 27
#define GROUP_BYTES (GROUP_SECTORS * SECTOR_BYTES)
#define MAX_MOVIES 64
#define PATH_CAPACITY 4096

typedef struct {
    char source[PATH_CAPACITY];
    char encoded[PATH_CAPACITY];
    char title[256];
    uint32_t frames;
    uint32_t groups;
    uint32_t lba;
} Movie;

typedef struct {
    void *process;
    void *output;
} MediaPipe;

void fail(const char *format, ...);
void *allocate(size_t size);
wchar_t *utf8_to_wide(const char *text);
FILE *open_file(const char *path, const char *mode);
int file_exists(const char *path);
uint64_t file_size(const char *path);
void make_directory(const char *path);
int run_process(const char *const *arguments);
MediaPipe open_media_pipe(const char *const *arguments);
size_t read_media_pipe(MediaPipe *pipe, void *data, size_t size);
int close_media_pipe(MediaPipe *pipe, int cancel);
void write_all(FILE *file, const void *data, size_t size);
void read_all(FILE *file, void *data, size_t size);
void put_be16(uint8_t *data, uint16_t value);
void put_be32(uint8_t *data, uint32_t value);
void put_le32(uint8_t *data, uint32_t value);
uint32_t get_be32(const uint8_t *data);
uint32_t get_le32(const uint8_t *data);
int find_movies(Movie *movies, const char *directory);
void encode_movie(Movie *movie, int force);
void write_video_header(FILE *file, uint32_t frames, uint32_t groups);
void read_video_header(Movie *movie);
void train_codebook(const uint8_t *tiles, int count, uint8_t *centers);
unsigned nearest_codeword(const uint8_t *tile, const uint8_t *centers);
unsigned tile_distance(const uint8_t *left, const uint8_t *right);
void encode_group(FILE *output, const uint8_t *rgb, const uint8_t *audio);
void make_font_and_titles(Movie *movies, int count);
void write_catalog(Movie *movies, int count);
void assemble_program(void);
void import_bootstrap(const char *path);
void validate_build_dependencies(void);
void create_disc(Movie *movies, int count);
void verify_disc(const char *path);
int run_self_tests(void);

#endif
