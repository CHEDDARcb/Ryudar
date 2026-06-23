# Ryudar 1.0

DirectX 11とC++17で実装した、3Dレンダラープロジェクトです。

既存のゲームエンジンには依存せず、Win32ウィンドウ、Direct3D 11の初期化、
GPUリソース管理、HLSLシェーダー、ライティング、モデル読み込み、
ポストプロセスまでを実装しています。

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

## デモ動画

全体の動作確認用動画は以下から確認できます。

[![Ryudar demo video](https://img.youtube.com/vi/rWNu4yBpQ5Q/maxresdefault.jpg)](https://youtu.be/rWNu4yBpQ5Q)

## 機能別デモ GIF

### Shading Model

![Shading model demo](https://github.com/user-attachments/assets/223ae696-c540-4e4b-9bd2-235eb8daddcb)

Phong と Blinn-Phong を切り替え、specular highlight の計算モデルを比較できます。

### Texture Toggle

![Texture toggle demo](https://github.com/user-attachments/assets/ebbdcdae-36cb-4e5d-a9fe-ea4def72e0ec)

Texture sampling の有効 / 無効を切り替え、material color と texture 適用結果を確認できます。

### Debug View

![Debug view demo](https://github.com/user-attachments/assets/950a5a5b-f78d-4261-8ad7-84060b6fadbc)

Normal 可視化と wireframe 表示で、mesh の向きや形状を確認できます。

### Object Transform / Selection

![Object transform demo](https://github.com/user-attachments/assets/d5e9f9c8-4d56-4997-b5c7-7304fe75fc56)

選択中の object に対して Translation、Rotation、Scale を個別に適用できます。

### Material Parameters

![Material parameters demo](https://github.com/user-attachments/assets/56540391-4165-434e-9925-185d2e546b2f)

Diffuse、Specular、Shininess を調整し、材質の反射特性を変更できます。

### Light Type

![Light type demo](https://github.com/user-attachments/assets/59c4e298-c683-4bb8-9911-ffc5ecfac839)

Directional、Point、Spot Light を切り替え、light type ごとの照明結果を確認できます。

### Rim Light

![Rim light demo](https://github.com/user-attachments/assets/4167d10f-6d2a-4c01-a10d-843619a4561c)

Rim Light により、object の輪郭付近を強調できます。

### Image Based Lighting

![Image based lighting demo](https://github.com/user-attachments/assets/4e0cba32-80c3-4a88-87a0-32b6177cc9e1)

IBL により、environment map を使った diffuse / specular lighting を適用できます。

### Post Processing

![Post processing demo](https://github.com/user-attachments/assets/5071617f-efec-4452-848c-ccf892af26b7)

Bloom の threshold、strength、blur を調整し、画面全体の発光表現を制御できます。

### First-Person Camera

![First-person camera demo](https://github.com/user-attachments/assets/347152c2-b8c0-46d4-a8db-ac5c6c442e2c)

WASD と mouse 操作で、scene 内を First-Person Camera として移動できます。

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
