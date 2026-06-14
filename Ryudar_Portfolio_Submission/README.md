Ryudar - DirectX 11 リアルタイムレンダリングアプリケーション
================================================

1. 提出物の構成
---------------
この提出フォルダーには、すぐに確認できる実行版と、実装内容を確認するための
ソースコード一式を収録しています。

・Run_Ryudar.bat
  提出フォルダー直下の実行用ファイルです。
  詳細な実行方法は「9. 実行方法」をご確認ください。

・01_Executable
  Release x64でビルドした実行版です。
  Ryudar.exe、実行用バッチファイル、必要なDLL、HLSL、Assetを収録しています。

・02_SourceCode
  Visual Studioソリューション、C++ / HLSLソース、実行用Assetを収録しています。
  BUILD.mdにはビルド手順、vcpkg.jsonには依存パッケージを記載しています。

・ThirdPartyLicenses
  使用したオープンソースライブラリのライセンス情報を収録しています。

2. プロジェクト概要
-------------------
Ryudarは、C++とDirectX 11を使用して実装したリアルタイム3Dレンダリングアプリケーションです。
既存のゲームエンジンには依存せず、Win32ウィンドウの生成、Direct3D 11の初期化、
GPUリソース管理、シェーダー、ライティング、ポストプロセスまでを実装しています。

ImGuiの操作パネルから描画方式やライト、マテリアル、モデルのTransform、
Bloomなどをリアルタイムに変更し、レンダリング結果を比較できます。

3. 主な機能
------------
・Phong / Blinn-Phongシェーディングの切り替え
・Directional Light / Point Light / Spot Light
・Diffuse、Specular、Shininessなどのマテリアル調整
・Image Based Lighting（Diffuse IBL / Specular IBL）
・キューブマップを利用したSkyboxとEnvironment Reflection
・Schlick近似によるFresnel反射
・Rim Light
・Texture / Wireframe / 頂点法線の表示切り替え
・SphereとFBXキャラクターモデルの切り替え
・First-Person Camera
・多段Downsample、Separable Blur、Upsample、CombineによるBloom
・ウィンドウリサイズ時のRender Target、Depth Buffer、Bloomリソース再生成

4. 操作方法
------------
右側の「Scene Control」ウィンドウから各レンダリング設定を変更できます。

・Use FPV: First-Person CameraのON / OFF
・W / S: 前進 / 後退（Use FPVがONの場合）
・A / D: 左移動 / 右移動（Use FPVがONの場合）
・マウス移動: カメラ方向の変更（Use FPVがONの場合）
・ESC: アプリケーション終了

主なGUI項目:
・Sphere / Character: 表示モデルの選択
・Shading Option: Phong / Blinn-Phongの選択
・Use Texture / Use Wireframe / Draw Normals: 描画デバッグ設定
・Use PostProc: Bloomの有効化、ThresholdとStrengthの調整
・Model: Translation、Rotation、Scalingの調整
・Material: Fresnel R0、Diffuse、Specular、Shininessの調整
・Light: Direct Light、IBL、Environment Reflection、Rim Lightの設定

5. レンダリングフロー
---------------------
1. CPU側で入力とImGuiの設定値を更新します。
2. Model / View / Projection、逆転置行列、ライト、マテリアルをConstant Bufferへ転送します。
3. Skyboxを描画します。
4. 選択中の3DモデルをClassic Litパイプラインで描画します。
5. MSAA Back Bufferをポストプロセス用のSingle-Sample TextureへResolveします。
6. 明るい領域を抽出し、段階的にDownsampleします。
7. Horizontal / Vertical BlurとUpsampleを繰り返してBloom画像を生成します。
8. 元画像とBloom画像を合成し、ImGuiを描画してPresentします。

6. ソフトウェア構成
-------------------
AppBase
  Win32メッセージループ、D3D11 Device / Context / Swap Chain、
  Render Target、Depth Buffer、ImGui、入力、リサイズ処理を管理します。

Ryudar
  シーン全体を管理するAppBaseの派生クラスです。
  モデル選択、GUI、ライト、マテリアル、IBL、Bloomパスを統合します。

ClassicLit::MeshGroup
  複数MeshのVertex / Index Buffer、Texture、Shader、Constant Bufferを管理します。
  通常描画と頂点法線のデバッグ描画を同じレンダリング単位で扱います。

CubeMapping
  Skybox描画と、各Meshが共有するDiffuse / Specular IBL Cubemapを管理します。

ImageFilter
  Full-Screen Quadを用いた1回分のポストプロセスPassを表します。
  Sampling、Horizontal Blur、Vertical Blur、Combineを共通インターフェースで接続します。

ModelLoader
  AssimpでFBXを読み込み、Node Transformを累積しながら独自のMeshDataへ変換します。
  Position、Normal、UV、Index、Diffuse Textureのパスを抽出します。

D3D11Utils
  Buffer、Texture、Shader、Input Layout、Depth Bufferなどの生成処理を共通化しています。
  HRESULT失敗時はD3D11Exceptionを通して処理内容とエラー情報を通知します。

7. 実装上のポイント
-------------------
・Microsoft::WRL::ComPtrを使用し、COMリソースの寿命をRAIIで管理しています。
・CPUとHLSLのConstant Buffer配置を合わせ、static_assertでサイズと16-byte境界を検証しています。
・Non-uniform Scaleでも法線を正しく変換するため、Model行列の逆転置行列を使用しています。
・DirectXのRow VectorとHLSL側の行列配置を意識し、GPU転送時にTransposeしています。
・GUI用のRenderSettingsとGPU転送用Constant Dataを分離しています。
・SphereとCharacterが個別のTransform、Material、描画設定を所有します。
・Bloomは固定処理ではなく、ImageFilterを連結するPass構成として実装しています。
・リサイズ前にRTV / SRVのBindingを解除し、Swap Chain関連リソースを再生成します。

8. 使用技術・ライブラリ
-----------------------
・C++17
・Win32 API
・DirectX 11 / DXGI / HLSL Shader Model 5.0
・DirectXTK / DirectXMath
・Dear ImGui
・Assimp
・stb_image
・vcpkg

外部ライブラリのライセンスはThirdPartyLicensesフォルダーに収録しています。

9. 実行方法
------------
1. 提出フォルダー直下のRun_Ryudar.batを実行します。
2. 01_ExecutableフォルダーのRyudar.exeを直接実行することもできます。

動作環境:
・Windows 10 / 11 64-bit
・DirectX 11 Feature Level 11.0対応GPU

Visual Studio、vcpkg、追加ライブラリのインストールは不要です。
実行に必要なVisual C++ RuntimeとDLLは同梱しています。
AssetとShaderは相対パスで読み込むため、01_Executable内の構成を維持してください。

10. ソースコードの確認・ビルド
------------------------------
詳細なビルド条件と依存パッケージは、同フォルダーのBUILD.mdおよびvcpkg.jsonに記載しています。

推奨確認箇所:
・Sources/Application/Ryudar.cpp: シーン、GUI、描画フロー、Bloom Pass構築
・Sources/Application/AppBase.cpp: Win32 / D3D11初期化、メインループ、リサイズ
・Sources/Rendering/ClassicLit: Mesh描画とレンダリング設定
・Sources/Rendering/ImageFilter.cpp: ポストプロセスの Pass
・Shaders/ClassicLitPixelShader.hlsl: Direct Light、IBL、Fresnel、Rim Light
・Shaders/BlurXPixelShader.hlsl / BlurYPixelShader.hlsl: Separable Blur
