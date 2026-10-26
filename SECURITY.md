# Security Policy

## Supported Versions

最新のリリース（`main` ブランチ）のみサポートします。

## Reporting a Vulnerability

脆弱性を見つけた場合は、公開の Issue ではなく
GitHub の **Security → Report a vulnerability**（Private vulnerability reporting）から報告してください。
内容を確認し、修正方針をご連絡します。

## Threat Model（想定する脅威）

| 対象 | 扱い |
| --- | --- |
| 細工されたセーブファイルによるメモリ破壊・クラッシュ | **対象**。読み込み時に全値を検証し、ファジングで継続的に確認 |
| 書き込み中の異常終了によるセーブ破損 | **対象**。一時ファイル経由のアトミック保存で防止 |
| ローカルでのセーブ改ざんによるチート | **対象外**。CRC32 は破損検知用であり、改ざん防止ではない |
| ネットワーク経由の攻撃 | **対象外**。ネットワーク機能を持たない |
