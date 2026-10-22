@echo off
rem SPDX-License-Identifier: GPL-3.0-only
rem Copyright (C) 2026 Retro-Orca
cd /d "%~dp0"
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1" %*
if errorlevel 1 exit /b 1
