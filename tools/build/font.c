/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#include "../common/common.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

static void render_mask(FILE *output, HDC dc, uint32_t *pixels, int width, int height, const wchar_t *text)
{
    RECT rectangle = {0, 0, width, height};
    FillRect(dc, &rectangle, GetStockObject(BLACK_BRUSH));
    SetTextColor(dc, RGB(255, 255, 255));
    SetBkColor(dc, RGB(0, 0, 0));
    TextOutW(dc, 0, -1, text, (int)wcslen(text));
    GdiFlush();
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; x += 8) {
            uint8_t bits = 0;
            for (int bit = 0; bit < 8; ++bit)
                if (pixels[y * 256 + x + bit] & 0xFFFFFF) bits |= (uint8_t)(0x80 >> bit);
            write_all(output, &bits, 1);
        }
}

void make_font_and_titles(Movie *movies, int count)
{
    BITMAPINFO info = {0};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = 256;
    info.bmiHeader.biHeight = -16;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    uint32_t *pixels = NULL;
    HDC dc = CreateCompatibleDC(NULL);
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, (void **)&pixels, NULL, 0);
    HFONT font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FIXED_PITCH, L"MS Gothic");
    if (!dc || !bitmap || !font) fail("Cannot create menu font");
    HGDIOBJ previous_bitmap = SelectObject(dc, bitmap);
    HGDIOBJ previous_font = SelectObject(dc, font);
    FILE *output = open_file("build/font.bin", "wb");
    for (int code = 32; code < 128; ++code) {
        wchar_t text[2] = {(wchar_t)code, 0};
        render_mask(output, dc, pixels, 8, 16, text);
    }
    fclose(output);
    for (int i = 0; i < count; ++i) {
        wchar_t title[256];
        MultiByteToWideChar(CP_UTF8, 0, movies[i].title, -1, title, 256);
        char path[80];
        snprintf(path, sizeof(path), "build/title-%03d.bin", i);
        output = open_file(path, "wb");
        render_mask(output, dc, pixels, 256, 16, title);
        fclose(output);
    }
    SelectObject(dc, previous_font);
    SelectObject(dc, previous_bitmap);
    DeleteObject(font);
    DeleteObject(bitmap);
    DeleteDC(dc);
}
