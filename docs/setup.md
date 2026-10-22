# 初回セットアップ

公開リポジトリは自作ソースと説明文書を含みます
外部ツールの実行ファイル、セガ由来の共通ブート領域、BIOS、動画、生成済みISOは含めません

## 1 Windows用Cコンパイラー

64ビットのMinGW-w64 GCCを用意します
たとえば [MSYS2](https://www.msys2.org/) のMINGW64環境を利用できます
GCCがPATHにない場合はフルパスを指定してください

```powershell
$env:HOST_CC = 'C:\msys64\mingw64\bin\gcc.exe'
.\build.cmd --test
```

このテストにはAS、FFmpeg、ブート領域、BIOSは不要です

## 2 Macro Assembler AS

[Macro Assembler AS公式](http://john.ccac.rwth-aachen.de:8000/as/) からWindows版を用意します
開発時にはAS V1.42 Beta Build 306を使用しました
`asw.exe` と同じ配布物のリソースファイルを `vendor/assembler` に配置します
別の場所を使う場合は実行ファイルのフルパスを指定します

```powershell
$env:SATURN_AS = 'C:\tools\as\asw.exe'
```

ASのライセンスとリソースファイルは入手元の配布物を保持してください

## 3 FFmpegとFFprobe

[FFmpeg公式](https://ffmpeg.org/download.html) からWindows用配布を選びます
デモ動画の生成にはlibx264とAACのエンコーダーが必要です
`ffmpeg.exe` と `ffprobe.exe` を `vendor/media` に置くか、PATHまたは環境変数で指定します

```powershell
$env:FFMPEG = 'C:\tools\ffmpeg\bin\ffmpeg.exe'
$env:FFPROBE = 'C:\tools\ffmpeg\bin\ffprobe.exe'
```

## 4 ローカル用ブート領域

このプレイヤーはサターンの共通ブート領域3456バイトを必要とします
そのデータの再配布許諾をこのプロジェクトからは与えないため、公開リポジトリには入れません
利用とコピーに必要な権利を持つIP.BINを自身で用意してください

日本、台湾、米国、欧州の4地域コードが `0xE00` から並ぶ配置に対応します
単一地域など配置が異なるIP.BINはインポート時に拒否します

```powershell
.\build.cmd --import-boot 'D:\your-authorized-files\IP.BIN'
```

入力ファイルは変更せず、`0x100..0xE7F` を `vendor/boot/saturn_common.bin` に保存します
この出力はGitの対象外です
取り込み後に動画を置いてビルドします

```powershell
.\build.cmd --demo
```

## 5 任意のエミュレーター確認

[Mednafen公式](https://mednafen.github.io/) からWindows版を用意します
`mednafen.exe` と同じ配布物のDLLを `build/emulator` に置くか、実行ファイルの場所を指定してください
プレビューの設定とセーブデータはこのプロジェクトの `build/emulator` に分けて保存します

```powershell
$env:MEDNAFEN_EXE = 'C:\tools\mednafen\mednafen.exe'
$env:SATURN_BIOS = 'D:\your-authorized-files\saturn-jp-bios.bin'
.\build\preview.exe
```

BIOSは利用者が用意したものを使用します
BIOSや共通ブート領域を入手する権利と、自分で生成したISOを再配布する権利は別に確認してください
