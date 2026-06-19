ソースコードを再ビルドする場合の案内
====================================

## 重要
この手順はソースコードを再ビルドする場合のみ必要です。
提出フォルダーのRun_Ryudar.batまたは01_Executable/Ryudar.exeを実行するだけであれば、
Visual Studio、vcpkg、開発用ライブラリのインストールは必要ありません。

## ビルド構成
・Release
・x64

## 再ビルドに必要なツール
・Visual Studio 2022以降
・Desktop development with C++ワークロード
・Windows 10 / 11 SDK
・vcpkg

## 依存パッケージ
・assimp
・directxtk
・imgui（dx11-binding、win32-binding）
・stb

vcpkg.jsonに直接使用する依存パッケージを記載しています。
Visual Studioとvcpkg integrationを設定した環境でRyudar.slnを開き、
Release | x64を選択してビルドできます。

実行時はAssetsとShadersを相対パスで読み込みます。
