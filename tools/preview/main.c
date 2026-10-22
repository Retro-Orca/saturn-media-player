/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Retro-Orca */

#define _UNICODE
#include <windows.h>
#include <wchar.h>
#include <stdio.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR arguments, int show)
{
    (void)instance; (void)previous; (void)arguments; (void)show;
    wchar_t root[4096];
    if (!GetModuleFileNameW(NULL, root, 4096)) return 1;
    wchar_t *end = wcsrchr(root, L'\\');
    if (!end) return 1;
    *end = 0;
    end = wcsrchr(root, L'\\');
    if (!end) return 1;
    *end = 0;
    wchar_t directory[4096], command[16384], bios[4096], emulator[4096];
    swprintf(directory, 4096, L"%ls\\build\\emulator", root);
    if (!GetEnvironmentVariableW(L"SATURN_BIOS", bios, 4096)) {
        MessageBoxW(NULL, L"環境変数SATURN_BIOSに日本向けBIOSファイルのパスを設定してください", L"Saturn preview", MB_OK);
        return 1;
    }
    if (!GetEnvironmentVariableW(L"MEDNAFEN_EXE", emulator, 4096))
        swprintf(emulator, 4096, L"%ls\\mednafen.exe", directory);
    CreateDirectoryW(directory, NULL);
    SetEnvironmentVariableW(L"MEDNAFEN_HOME", directory);
    SetEnvironmentVariableW(L"MEDNAFEN_NOPOPUPS", L"1");
    swprintf(command, 16384,
        L"\"%ls\" -ss.bios_jp \"%ls\" -ss.region_autodetect 0 -ss.region_default jp -ss.cart none "
        L"-video.driver softfb -video.fs 0 -ss.xscale 2 -ss.yscale 2 -sound.driver dsound "
        L"-ss.input.port1 gamepad -ss.input.port1.gamepad.a \"keyboard 0x0 4\" "
        L"-ss.input.port1.gamepad.up \"keyboard 0x0 82\" -ss.input.port1.gamepad.down \"keyboard 0x0 81\" "
        L"-ss.input.port1.gamepad.left \"keyboard 0x0 80\" -ss.input.port1.gamepad.right \"keyboard 0x0 79\" "
        L"-ss.input.port1.gamepad.start \"keyboard 0x0 40\" -ss.input.port1.gamepad.x \"keyboard 0x0 27\" "
        L"-ss.input.port1.gamepad.y \"keyboard 0x0 28\" -ss.input.port1.gamepad.z \"keyboard 0x0 29\" "
        L"-ss.input.port1.gamepad.ls \"keyboard 0x0 20\" -ss.input.port1.gamepad.rs \"keyboard 0x0 8\" "
        L"\"%ls\\build\\saturn-video.cue\"", emulator, bios, root);
    STARTUPINFOW startup = {0};
    PROCESS_INFORMATION process = {0};
    startup.cb = sizeof(startup);
    if (!CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, directory, &startup, &process)) {
        MessageBoxW(NULL, L"build/emulatorのMednafenとSATURN_BIOSの設定を確認してください", L"Saturn preview", MB_OK);
        return 1;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return 0;
}
