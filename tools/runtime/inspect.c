/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HANDLE process;
static uintptr_t ram;
static int byte_swap;
static uint8_t expected_code[64];
static int search_swap;

static int read_memory(uintptr_t address, void *buffer, size_t size)
{
    SIZE_T actual = 0;
    return ReadProcessMemory(process, (const void *)address, buffer, size, &actual) && actual == size;
}

static uintptr_t find_pattern(const uint8_t *pattern, size_t length)
{
    MEMORY_BASIC_INFORMATION region;
    uintptr_t address = 0;
    uint8_t *buffer = malloc(1024 * 1024 + length);
    if (!buffer) return 0;
    while (VirtualQueryEx(process, (void *)address, &region, sizeof(region))) {
        uintptr_t base = (uintptr_t)region.BaseAddress;
        size_t size = region.RegionSize;
        if (region.State == MEM_COMMIT && !(region.Protect & (PAGE_GUARD | PAGE_NOACCESS)) &&
            (region.Protect & 0xEE) && size <= 128 * 1024 * 1024) {
            for (size_t offset = 0; offset < size; offset += 1024 * 1024) {
                size_t bytes = size - offset;
                if (bytes > 1024 * 1024 + length) bytes = 1024 * 1024 + length;
                if (!read_memory(base + offset, buffer, bytes)) continue;
                for (size_t i = 0; i + length <= bytes; ++i)
                    if (buffer[i] == pattern[0] && !memcmp(buffer + i, pattern, length)) {
                        if (length == 8) {
                            uint8_t check[64];
                            uintptr_t candidate = base + offset + i - 0xD0060 + 0x4000;
                            if (!read_memory(candidate, check, sizeof(check))) continue;
                            int valid = 1;
                            for (int j = 0; j < 64; ++j) if (check[j ^ search_swap] != expected_code[j]) { valid = 0; break; }
                            if (!valid) continue;
                        }
                        free(buffer);
                        return base + offset + i;
                    }
            }
        }
        if (!size || base + size <= address) break;
        address = base + size;
    }
    free(buffer);
    return 0;
}

static uint32_t state_word(const uint8_t *data, unsigned offset)
{
    uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i) value = value << 8 | data[(offset + i) ^ byte_swap];
    return value;
}

static int read_state(uint8_t *data)
{
    return read_memory(ram + 0xD0000, data, 128);
}

static void print_state(void)
{
    uint8_t data[128];
    if (!read_state(data)) { puts("Cannot read player state"); return; }
    printf("{\"mode\":%u,\"selected\":%u,\"group\":%u,\"frame_in_group\":%u,\"frame\":%u,\"filled_group\":%u,\"fill_sector\":%u,\"vblanks\":%u,\"cd_error\":%u,\"audio_error\":%u,\"underruns\":%u,\"pad\":%u,\"timer_ticks\":%u}\n",
        state_word(data, 12), state_word(data, 8), state_word(data, 20), state_word(data, 24), state_word(data, 28),
        state_word(data, 32), state_word(data, 36), state_word(data, 60), state_word(data, 64), state_word(data, 76), state_word(data, 80), state_word(data, 0), state_word(data, 104));
    fflush(stdout);
}

static uintptr_t find_menu_vram(void)
{
    uint8_t font[1536], pattern[304];
    FILE *file = fopen("build/font.bin", "rb");
    if (!file) return 0;
    size_t count = fread(font, 1, sizeof(font), file); fclose(file);
    if (count != sizeof(font)) return 0;
    const char *text = "SATURN VIDEO PLAYER";
    for (size_t c = 0; c < strlen(text); ++c) {
        uint8_t row = font[(text[c] - 32) * 16 + 3];
        for (int bit = 0; bit < 8; ++bit) {
            uint16_t color = (row & (0x80 >> bit)) ? 0xFFE0 : 0x8000;
            unsigned p = (unsigned)c * 16 + bit * 2;
            pattern[p ^ byte_swap] = (uint8_t)(color >> 8);
            pattern[(p + 1) ^ byte_swap] = (uint8_t)color;
        }
    }
    uintptr_t match = find_pattern(pattern, strlen(text) * 16);
    return match ? match - (11 * 1024 + 32) : 0;
}

static void dump_bitmap(uintptr_t vram, const char *path)
{
    uint8_t *data = malloc(262144);
    if (!data || !read_memory(vram, data, 262144)) { free(data); return; }
    FILE *file = fopen(path, "wb");
    if (!file) { free(data); return; }
    fprintf(file, "P6\n320 224\n255\n");
    for (int y = 0; y < 224; ++y)
        for (int x = 0; x < 320; ++x) {
            unsigned p = y * 1024 + x * 2;
            unsigned color = data[p ^ byte_swap] << 8 | data[(p + 1) ^ byte_swap];
            uint8_t rgb[3] = {(uint8_t)((color & 31) * 255 / 31), (uint8_t)(((color >> 5) & 31) * 255 / 31), (uint8_t)(((color >> 10) & 31) * 255 / 31)};
            fwrite(rgb, 1, 3, file);
        }
    fclose(file); free(data);
}

static void dump_tiles(uintptr_t vram, const char *path)
{
    uint8_t state[128];
    if (!vram || !read_state(state)) return;
    uint8_t *data = malloc(524288);
    if (!data || !read_memory(vram, data, 524288)) { free(data); return; }
    unsigned map = 0x40000 + ((state_word(state, 52) ^ 1) & 1) * 16384;
    FILE *file = fopen(path, "wb");
    if (!file) { free(data); return; }
    fprintf(file, "P6\n320 224\n255\n");
    for (int y = 0; y < 224; ++y)
        for (int x = 0; x < 320; ++x) {
            unsigned p = map + (y / 8 * 64 + x / 8) * 4 + 2;
            unsigned character = (data[p ^ byte_swap] << 8 | data[(p + 1) ^ byte_swap]) & 32767;
            unsigned pixel = character * 32 + (y % 8 * 8 + x % 8) * 2;
            unsigned color = data[pixel ^ byte_swap] << 8 | data[(pixel + 1) ^ byte_swap];
            uint8_t rgb[3] = {(uint8_t)((color & 31) * 255 / 31), (uint8_t)(((color >> 5) & 31) * 255 / 31), (uint8_t)(((color >> 10) & 31) * 255 / 31)};
            fwrite(rgb, 1, 3, file);
        }
    fclose(file); free(data);
}

static void press_button(uint32_t button)
{
    uint8_t request[4];
    for (int i = 0; i < 4; ++i) request[i ^ byte_swap] = (uint8_t)(button >> (24 - i * 8));
    SIZE_T actual = 0;
    if (!WriteProcessMemory(process, (void *)(ram + 0xD0054), request, 4, &actual) || actual != 4) {
        puts("Cannot write the test input mailbox"); exit(1);
    }
}

static uint32_t sample_word(unsigned offset)
{
    uint8_t state[128];
    if (!read_state(state)) { puts("Player state became unavailable"); exit(1); }
    return state_word(state, offset);
}

static void expect_mode(unsigned mode)
{
    for (int i = 0; i < 100; ++i) {
        if (sample_word(12) == mode) return;
        if (sample_word(64)) { print_state(); puts("CD operation failed"); exit(1); }
        Sleep(50);
    }
    print_state(); puts("Timed out waiting for player mode"); exit(1);
}

static void exercise_player(uintptr_t vram)
{
    press_button(0x0800);
    expect_mode(1);
    Sleep(250);
    uint8_t before[128], after[128];
    read_state(before);
    Sleep(1500);
    read_state(after);
    uint32_t frames = state_word(after, 28) - state_word(before, 28), ticks = state_word(after, 104) - state_word(before, 104);
    if (frames < 60 || !ticks) { print_state(); puts("Playback did not advance"); exit(1); }
    printf("Playback timing: %u frames, %u timer ticks, %.2f ticks/frame\n", frames, ticks, (double)ticks / frames);
    printf("Frame interval: last %u ticks, max %u ticks, late intervals %u\n", sample_word(112), sample_word(116), sample_word(120));
    print_state();
    dump_tiles(vram, "build/playback.ppm");
    press_button(2);
    expect_mode(2);
    uint32_t paused_frame = sample_word(28);
    Sleep(400);
    if (sample_word(28) != paused_frame) { puts("Y pause failed"); exit(1); }
    press_button(1);
    expect_mode(1);
    Sleep(250);
    if (sample_word(28) <= paused_frame) { puts("Z resume failed"); exit(1); }
    press_button(1);
    expect_mode(2);
    paused_frame = sample_word(28);
    Sleep(200);
    if (sample_word(28) != paused_frame) { puts("Z pause failed"); exit(1); }
    press_button(4);
    expect_mode(0);
    Sleep(100);
    press_button(0x0800);
    expect_mode(1);
    press_button(0x8000);
    expect_mode(4);
    if (sample_word(20) < 25) { puts("L seek failed"); exit(1); }
    expect_mode(1);
    press_button(4);
    expect_mode(0);
    Sleep(100);
    press_button(0x0800);
    expect_mode(1);
    press_button(8);
    expect_mode(4);
    if (sample_word(20) < 29) { puts("R seek failed"); exit(1); }
    expect_mode(1);
    expect_mode(0);
    if (sample_word(80) || sample_word(76)) { print_state(); puts("Audio initialization or stream underrun failed"); exit(1); }
    printf("Final frame intervals: max %u ticks, late intervals %u\n", sample_word(116), sample_word(120));
    if (sample_word(120)) { puts("A video frame missed its display interval"); exit(1); }
    print_state();
    puts("PASS: start, playback, Y pause, Z resume/pause, X list, L/R seek, end of movie, zero underruns");
}

static void exercise_menu(uintptr_t vram)
{
    for (unsigned i = 1; i <= 10; ++i) {
        press_button(32);
        Sleep(90);
        if (sample_word(8) != i % 10) { print_state(); puts("Menu selection failed"); exit(1); }
        if (i == 9) dump_bitmap(vram, "build/menu-japanese.ppm");
    }
    press_button(16);
    Sleep(90);
    if (sample_word(8) != 9) { puts("Menu up wrap failed"); exit(1); }
    press_button(32);
    Sleep(90);
    if (sample_word(8)) { puts("Menu down wrap failed"); exit(1); }
    puts("PASS: 10 video entries, page change, up/down wrap, Japanese title bitmap");
}

int main(int argc, char **argv)
{
    if (argc < 2) { puts("Usage: inspect.exe PID [--exercise]"); return 1; }
    int exercise = argc > 2 && !strcmp(argv[2], "--exercise");
    int menu_test = argc > 2 && !strcmp(argv[2], "--menu-test");
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | ((exercise || menu_test) ? PROCESS_VM_WRITE | PROCESS_VM_OPERATION : 0), FALSE, (DWORD)strtoul(argv[1], NULL, 10));
    if (!process) { printf("OpenProcess failed: %lu\n", GetLastError()); return 1; }
    uint8_t pattern[8];
    FILE *file = fopen("build/1ST.BIN", "rb");
    if (!file || fread(expected_code, 1, sizeof(expected_code), file) != sizeof(expected_code)) return 1;
    fclose(file);
    for (int attempt = 0; attempt < 20 && !ram; ++attempt) {
        for (int swap = 0; swap < 2; ++swap) {
            search_swap = swap;
            for (int i = 0; i < 8; ++i) pattern[i] = ((const uint8_t *)"SVT1STAT")[i ^ swap];
            uintptr_t found = find_pattern(pattern, sizeof(pattern));
            if (found) { ram = found - 0xD0060; byte_swap = swap; break; }
        }
        if (!ram) Sleep(1000);
    }
    if (!ram) {
        for (int swap = 0; swap < 2; ++swap) {
            uint8_t probe[64];
            for (int i = 0; i < 64; ++i) probe[i] = expected_code[i ^ swap];
            uintptr_t found = find_pattern(probe, sizeof(probe));
            if (found) {
                printf("Executable image found at %llx (swap %d), runtime marker absent\n", (unsigned long long)found, swap);
                if (swap == 1) {
                    ram = found - 0x4000; byte_swap = swap; print_state();
                    uint8_t bytes[16];
                    if (read_memory(ram + 0xD0060, bytes, sizeof(bytes))) {
                        for (int j = 0; j < 16; ++j) printf("%02x", bytes[j ^ swap]);
                        puts("");
                    }
                }
            }
        }
        puts("Player executable not found in emulated RAM"); return 1;
    }
    print_state();
    uintptr_t vram = find_menu_vram();
    if (vram) { dump_bitmap(vram, "build/menu.ppm"); printf("Menu VRAM: %llx\n", (unsigned long long)vram); }
    if (exercise) {
        exercise_player(vram);
    }
    if (menu_test) exercise_menu(vram);
    CloseHandle(process);
    return 0;
}
