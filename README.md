# Ryudar 1.0

DirectX 11とC++17で実装した、3Dレンダラープロジェクトです。

既存のゲームエンジンには依存せず、Win32ウィンドウ、Direct3D 11の初期化、
GPUリソース管理、HLSLシェーダー、ライティング、モデル読み込み、
ポストプロセスまでを実装しています。

## デモ GIF

> ここにプロジェクトの動作が分かる GIF を配置します。
>
> 推奨配置: `Docs/Images/ryudar_demo.gif`
>
> README への埋め込み例:
>
> `![Ryudar demo](./Docs/Images/ryudar_demo.gif)`

この GIF では、ライティング切り替え、IBL、Skybox、Bloom、ImGui による
リアルタイムパラメータ調整が一目で分かる流れを見せると効果的です。

## 主な機能

- Phong / Blinn-Phongシェーディング
- Directional / Point / Spot Light
- Image Based LightingとEnvironment Reflection
- Cubemapを利用したSkybox
- Fresnel反射とRim Light
- FBXモデル読み込み
- Texture / Wireframe / Normal可視化
- First-Person Camera
- 多段DownsampleとSeparable BlurによるBloom
- ImGuiによるリアルタイムパラメーター編集
- ウィンドウリサイズ時のGPUリソース再生成

## 実行方法

ビルド環境を用意せずに実行できるRelease版を同梱しています。

1. [`Ryudar_Portfolio_Submission`](./Ryudar_Portfolio_Submission)を開きます。
2. `Run_Ryudar.bat`を実行します。

Visual Studio、vcpkg、追加ライブラリのインストールは不要です。
必要なDLL、Shader、Asset、Visual C++ Runtimeは実行版に同梱しています。

## 操作

- `W` / `S`: 前進 / 後退（First-Person Camera有効時）
- `A` / `D`: 左移動 / 右移動（First-Person Camera有効時）
- マウス移動: カメラ方向変更（First-Person Camera有効時）
- `ESC`: 終了
- `Scene Control`: モデル、ライト、マテリアル、Bloomなどの設定

## プロジェクト構成

- `Sources`: C++実装
- `Headers`: ヘッダーとデータ構造
- `Shaders`: HLSLシェーダー
- `Assets`: Texture、Cubemap、FBXモデル
- `Ryudar_Portfolio_Submission`: 実行版、提出用ソース、ライセンス

詳細な機能、設計、レンダリングフローについては、
提出用の[README](./Ryudar_Portfolio_Submission/README.md)をご確認ください。

## 使用技術

- C++17
- Win32 API
- DirectX 11 / DXGI
- HLSL Shader Model 5.0
- DirectXTK / DirectXMath
- Dear ImGui
- Assimp
- stb_image
- vcpkg

## ソースコードの確認

主な実装:

- [`Sources/Application/Ryudar.cpp`](./Sources/Application/Ryudar.cpp): シーンと描画フロー
- [`Sources/Application/AppBase.cpp`](./Sources/Application/AppBase.cpp): Win32 / D3D11基盤
- [`Sources/Rendering/ClassicLit`](./Sources/Rendering/ClassicLit): Mesh描画とShading
- [`Sources/Rendering/ImageFilter.cpp`](./Sources/Rendering/ImageFilter.cpp): Bloom Pass
- [`Shaders/ClassicLitPixelShader.hlsl`](./Shaders/ClassicLitPixelShader.hlsl): Light、IBL、Fresnel
