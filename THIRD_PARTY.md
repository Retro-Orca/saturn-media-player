# 利用した既存ファイル

公開リポジトリに含むのは自作ソースと文書です
以下の外部バイナリ、共通ブート領域、BIOS、生成したディスクイメージはGitの公開対象から除外しています

## Macro Assembler AS

開発時はAS V1.42 Beta Build 306を使用しました
AS本体を変更せず別プロセスとして呼び出します
入手したASのGNU GPLライセンス文書とリソースを利用者の手元で保持してください

配布元とソースは [Macro Assembler AS](http://john.ccac.rwth-aachen.de:8000/as/) を参照してください

## FFmpeg / FFprobe

開発時はGyanのFFmpeg 9.0 full buildを使用しました
FFmpeg本体を変更せず、別プロセスとして呼び出します

ソースとライセンスは [FFmpeg](https://ffmpeg.org/) と [Gyanのビルド配布](https://www.gyan.dev/ffmpeg/builds/) にあります
このfull buildはGPL機能を有効にしているため、外部へ配布する場合は対応するライセンスとソース提供条件を確認してください

## サターンの共通ブート領域

開発時に `vendor/boot/saturn_common.bin` として利用したのは標準ブート共通部の3456バイトです
Jo Engineの `Compiler/COMMON/IP.BIN` の `0x100..0xE7F` と一致します
セキュリティ領域と地域コードを含む既存バイナリで、このプロジェクトの自作コードとしては扱いません
Jo Engine自体のMITライセンスが、セガ由来のこの領域について再配布許諾を与えているとは断定していません
このプロジェクトではそのデータを配布せず、利用者が用意したIP.BINからローカルに取り込む方法を提供します
アプリケーション初期化入口は `src/boot/entry.asm` から自前でアセンブルします

仕様は [Segaのブート規約](https://www.jo-engine.org/upload/files/SEGA/ST-TECH-1-41.pdf) を参照してください

## 検証用Mednafen

開発時の検証にはMednafen 1.32.1と依存DLLを使用しました
実機用ソフトにはリンクせず、動作検証だけに使用します
Mednafenを用意する場合は配布物の `COPYING` と依存DLLの条件を保持してください
BIOSはこの環境の利用者が用意したファイルを参照し、プロジェクトへコピーしていません

[Mednafen公式](https://mednafen.github.io/) / [サターンの説明](https://mednafen.github.io/documentation/ss.html)
