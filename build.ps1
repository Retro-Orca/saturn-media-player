# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Retro-Orca

param([Parameter(ValueFromRemainingArguments=$true)][string[]]$BuildArguments)
$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath $PSScriptRoot
try {
    $compiler = $env:HOST_CC
    if (-not $compiler) {
        $candidate = Get-Command gcc -ErrorAction SilentlyContinue
        if ($candidate) { $compiler = $candidate.Source }
    }
    if (-not $compiler -and (Test-Path -LiteralPath 'C:\msys64\mingw64\bin\gcc.exe')) {
        $compiler = 'C:\msys64\mingw64\bin\gcc.exe'
    }
    if (-not $compiler) { throw 'MinGW-w64 GCCをインストールするかHOST_CCにgcc.exeを指定してください' }
    $env:PATH = (Split-Path -Parent $compiler) + ';' + $env:PATH
    New-Item -ItemType Directory -Path build -Force | Out-Null
    $sources = @(Get-ChildItem -LiteralPath tools -Recurse -Filter '*.c' | Where-Object { $_.Directory.Name -notin @('preview','runtime') } | ForEach-Object FullName)
    $sources += Join-Path $PSScriptRoot 'tests\codec.c'
    & $compiler -std=c11 -O3 -Wall -Wextra -Werror -msse2 -municode -static $sources -lgdi32 -o build\saturn-build.exe
    if ($LASTEXITCODE -ne 0) { throw 'Cビルドツールのコンパイルに失敗しました' }
    & $compiler -std=c11 -O2 -Wall -Wextra -Werror -municode -mwindows -static tools\preview\main.c -o build\preview.exe
    if ($LASTEXITCODE -ne 0) { throw 'プレビュー起動アプリのコンパイルに失敗しました' }
    & .\build\saturn-build.exe @BuildArguments
    if ($LASTEXITCODE -ne 0) { throw "ビルドに失敗しました ($LASTEXITCODE)" }
} finally {
    Pop-Location
}
