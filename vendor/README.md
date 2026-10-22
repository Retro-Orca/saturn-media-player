# 外部依存ファイルの配置先

このフォルダーには外部の実行ファイルや共通ブート領域を同梱しません
利用するツールは各配布元から入手し、[セットアップ手順](../docs/setup.md) に従って配置してください

```text
vendor/
  assembler/   asw.exeと同じ配布物のリソースファイル
  boot/        利用者が取り込んだsaturn_common.bin
  media/       ffmpeg.exeとffprobe.exe
```

これらのローカルファイルは `.gitignore` で公開対象から除外しています
自作コードのGPLv3ライセンスは外部の実行ファイルやブート領域には適用しません
