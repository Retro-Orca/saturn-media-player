# Saturn Video Player

好きな動画からセガサターン用の起動ディスクを作るプレイヤーです
実機側の自作コードはすべてSH-2アセンブラ、PC側の動画変換とディスク生成はCで書いています

自作ソースはGPLv3で公開します
初めて使う場合は [セットアップ手順](docs/setup.md) を確認してください
外部ツール、共通ブート領域、BIOS、動画、生成済みISOはリポジトリに含めません

## 使い方

1. `videos` に動画を入れる
2. `build.cmd` を実行する
3. `build/saturn-video.cue` と `build/saturn-video.iso` を同じ場所に置いて起動する

MP4、MKV、MOV、AVI、WebM、M4V、MPEG、MPGに対応します
動画名はファイル名から作り、日本語と空白を含む名前も扱えます
最大64本を名前順に表示し、一覧は8本ごとに切り替わります

```powershell
# 6秒のテスト動画でディスクを作る
.\build.cmd --demo

# videos内の動画でディスクを作る
.\build.cmd

# 変換済みデータを作り直す
.\build.cmd --reencode

# 圧縮形式の検査
.\build.cmd --test

# 生成したディスクの検査
.\build\saturn-build.exe --verify
```

ビルド済みの `build/saturn-build.exe` を直接実行することもできます
変換済み動画は入力ファイルのハッシュで再利用します
FFmpegから少しずつ読み込むため、動画全体の非圧縮一時ファイルは作りません

## 操作

| ボタン | 動作 |
| --- | --- |
| 上下 | 一覧の選択 |
| A / START | 選択した動画を最初から再生 |
| X | 再生中、停止中、読み込み中から動画一覧へ戻る |
| Y | 一時停止 |
| Z | 再生と一時停止を切り替える |
| L | 5秒先へ進む |
| R | 30秒先へ進む |

動画の最後まで再生すると一覧に戻ります
スキップ先が終端を超える場合は最後の0.2秒単位の区間へ移動します

## 映像と音声

- 表示は320×224、RGB555、NTSCの垂直同期ごとに1フレーム
- 入力動画は60fpsに変換し、実際の表示速度はNTSC本体の約59.8Hzに従う
- 4×4の小画像384種類と変化部分の更新を使う専用VQ圧縮
- 音声は22,050Hz、16ビット、モノラルPCM
- 音声は循環バッファで連続再生し、0.2秒ごとの再スタートを行わない
- 映像、音声、セクターの余白を合わせて毎秒276,480バイト
- 元の縦横比を維持し、必要な部分に黒帯を付ける

一般的なMP4をそのまま実機でデコードする方式ではありません
CDの帯域に収める非可逆圧縮なので、細かい模様や激しい動きにはブロック状の劣化が出ます
低いフレームレートの元動画はフレームを複製して60fpsにし、中間の動きは生成しません
画質とデータ配置の詳細は [動画形式](docs/format.md) に記載しています

## 動作環境

日本向けNTSCサターンを基準にしています
実機ではODEなど自作ディスクを起動できる環境が必要です
PAL本体での60fps再生と実機での動作確認は未完了です

ビルドには64ビットWindows、MinGW-w64 GCC、FFmpeg、FFprobe、Macro Assembler ASを使います
外部ツールは利用者が用意します
GCCはPATH、`HOST_CC`、`C:/msys64/mingw64/bin/gcc.exe` の順に探します

```powershell
$env:HOST_CC = 'C:\your-toolchain\bin\gcc.exe'
$env:FFMPEG = 'C:\your-tools\ffmpeg.exe'
$env:FFPROBE = 'C:\your-tools\ffprobe.exe'
$env:SATURN_AS = 'C:\your-tools\asw.exe'
.\build.cmd
```

## エミュレーターで試す

ビルドで生成した `build/preview.exe` からプレビューできます
Mednafenと依存DLLは `build/emulator` に配置するか `MEDNAFEN_EXE` で指定してください
BIOSは利用者が用意したファイルを使い、ディスクには含めません
環境変数 `SATURN_BIOS` に日本向けBIOSのパスを設定します

プレビューのキーはA、Enter、X、Y、Z、QがL、EがRです
上下は矢印キー、F12でエミュレーターを終了します
音声デバイスはDirectSoundを使用します

## ソースの構成

| フォルダー | 内容 |
| --- | --- |
| `src/boot` | 自作の起動入口 |
| `src/platform` | ハードウェア定義とメモリー転送 |
| `src/cd` | CDコマンドと連続読み込み |
| `src/player` | 先読み、再生状態、操作 |
| `src/video` | VDP2初期化、辞書、パターン表、時間計測 |
| `src/audio` | SCSP初期化とPCM再生 |
| `src/input` | デジタルパッド入力 |
| `src/ui` | 一覧、文字、読み込み画面 |
| `tools/encode` | CによるVQ圧縮 |
| `tools/disc` | CによるISO9660生成 |
| `tools/build` | アセンブル、フォント、動画カタログ |
| `tools/boot` | 利用者が用意したブート領域の取り込み |
| `tools/common` | ファイル、Unicode、外部プロセス、パイプ |
| `tools/verify` | ディスク構造の検査 |
| `tools/runtime` | Mednafen内の状態を検証するCツール |
| `tools/preview` | Cで書いたプレビュー起動アプリ |
| `tests` | 圧縮とデータ配置のテスト |

検証結果は [検証メモ](docs/verification.md)、既存ツールとブート共通部の出典は [THIRD_PARTY.md](THIRD_PARTY.md) を参照してください

## ライセンス

Copyright (C) 2026 Retro-Orca

このプロジェクトの自作ソースはGNU General Public License version 3に従って利用、改変、再配布できます
無保証です
ライセンスの全文は [LICENSE](LICENSE) にあります
外部ツール、セガの共通ブート領域、BIOS、利用者が入力する動画の権利はそれぞれの権利者に帰属します
