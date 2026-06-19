# 해당 내용은 노션에서 확인 가능

# Ryudar プロジェクト構造説明

この文書は、面接官または採用担当者が Ryudar プロジェクト全体を確認する前に、ソースコードの構成、主要クラスの役割、レンダリング処理の流れを把握するための事前説明資料です。

コードを読む際は、まずこの文書で全体像を掴んだうえで、`Application`、`Rendering`、`Geometry`、`Scene`、`Graphics` の順に確認すると構造を追いやすくなります。

## ダイアグラム

構造説明用の図は `Docs/ArchitectureDiagrams` に保存されています。

- 図 1: [`01_project_layers.svg`](./ArchitectureDiagrams/01_project_layers.svg)  
  プロジェクト全体のレイヤー構造
- 図 2: [`02_class_relationships.svg`](./ArchitectureDiagrams/02_class_relationships.svg)  
  主要クラスとデータ構造の関係
- 図 3: [`03_frame_render_flow.svg`](./ArchitectureDiagrams/03_frame_render_flow.svg)  
  1 フレーム単位の実行・描画フロー

## プロジェクト概要

Ryudar は DirectX 11 / C++17 で作成されたリアルタイム 3D レンダリングデモです。

主な機能は以下です。

- Win32 / Direct3D 11 によるアプリケーション基盤
- HLSL によるシェーダー描画
- Phong / Blinn-Phong ライティング
- Directional / Point / Spot Light
- Material パラメータ編集
- Image Based Lighting
- Environment Reflection
- Cubemap Skybox
- FBX モデル読み込み
- Procedural Geometry 生成
- Texture / Wireframe / Normal Debug 表示
- First-Person Camera
- Bloom Post Processing
- ImGui によるリアルタイム GUI 操作

プロジェクトの大まかな構成は、DirectX 11 の初期化とフレームループを担当する `AppBase`、実際のデモシーンを構築する `Ryudar`、メッシュ描画を担当する `ClassicLit::MeshGroup`、環境マップを扱う `CubeMapping`、後処理を担当する `ImageFilter` を中心に組まれています。

## フォルダ構成

```text
Headers
  C++ ヘッダー

Sources
  C++ 実装ファイル

Shaders
  HLSL シェーダー

Assets
  テクスチャ、Cubemap、FBX モデルなど

Docs
  プロジェクト構造説明とダイアグラム

Ryudar_Portfolio_Submission
  提出用にまとめた実行ファイル・ソース一式
```

実際の開発用ソースは、主に `Headers`、`Sources`、`Shaders`、`Assets` にあります。`Ryudar_Portfolio_Submission` は配布・提出用のコピーとして扱われます。

## レイヤー構造

プロジェクト全体は、以下のようなレイヤーに分けて理解できます。

```text
Application Layer
  main.cpp
  AppBase
  Ryudar

Scene Layer
  SceneObject
  Transform
  Material
  Light

Rendering Layer
  Camera
  ClassicLit::MeshGroup
  CubeMapping
  ImageFilter

Geometry / Asset Layer
  Vertex
  MeshData
  Mesh
  GeometryGenerator
  ModelLoader

Graphics Infrastructure
  D3D11Utils
  D3D11Exception

GPU Shader Layer
  Shaders/*.hlsl
```

`Application Layer` はアプリケーションの起動、ウィンドウ、フレームループを管理します。

`Scene Layer` はオブジェクトの状態、Transform、Material、Light などのシーン情報を表します。

`Rendering Layer` は実際の描画処理を担当します。

`Geometry / Asset Layer` は CPU 側のメッシュデータ生成や外部モデル読み込みを担当します。

`Graphics Infrastructure` は Direct3D 11 のリソース生成やエラー処理を共通化します。

`GPU Shader Layer` は HLSL による GPU 側の描画処理です。

## 実行開始地点

エントリーポイントは `Sources/Application/main.cpp` です。

`main.cpp` は `Ryudar` アプリケーションを生成し、初期化後にメインループを開始します。

```cpp
Ryudar::Ryudar ryudarApp;

if (!ryudarApp.Initialize())
{
    return EXIT_FAILURE;
}

return ryudarApp.Run();
```

実行の流れは以下です。

```text
main.cpp
  -> Ryudar instance 作成
  -> Ryudar::Initialize()
  -> AppBase::Run()
```

## AppBase

関連ファイル:

- `Headers/Application/AppBase.h`
- `Sources/Application/AppBase.cpp`

`AppBase` はアプリケーション基盤を担当する基底クラスです。

主な責務は以下です。

- Win32 window class 登録
- メインウィンドウ作成
- Direct3D 11 device / context 作成
- swap chain 作成
- render target view 作成
- depth stencil view / state 作成
- rasterizer state 作成
- ImGui 初期化
- Win32 message loop 実行
- keyboard / mouse 入力状態の管理
- window resize 時の D3D resource 再生成
- 共通 camera の保持

`AppBase` は以下の純粋仮想関数を持っています。

```cpp
virtual void UpdateGUI() = 0;
virtual void Update(float dt) = 0;
virtual void Render() = 0;
```

そのため、`AppBase` はアプリケーション共通処理を提供し、具体的なシーン更新と描画は派生クラスで実装する構造です。

`Ryudar` はこの `AppBase` を継承して、デモ固有の GUI、更新、描画処理を実装しています。

## Ryudar

関連ファイル:

- `Headers/Application/Ryudar.h`
- `Sources/Application/Ryudar.cpp`

`Ryudar` は本プロジェクトのメインアプリケーションクラスです。

```cpp
class Ryudar : public AppBase
```

主な責務は以下です。

- シーンリソースの初期化
- sphere object の生成
- character model の読み込み
- cubemap / IBL resource の初期化
- bloom post-process filter chain の構築
- ImGui によるシーン制御 UI
- camera、light、material、transform の更新
- constant buffer の更新
- skybox、mesh、post-process の描画順制御

主要メンバーは以下です。

```cpp
SceneObject m_sphere;
SceneObject m_character;
CubeMapping m_cubeMapping;

LightType m_selectedLightType;
Light m_editableLight;

MeshType m_selectedMeshType;

std::vector<std::unique_ptr<ImageFilter>> m_filters;
```

`m_sphere` と `m_character` はそれぞれ独立した `SceneObject` です。GUI で `MeshType` を切り替えることで、現在表示するオブジェクトを選択します。

```cpp
enum class MeshType
{
    Sphere,
    Character,
};
```

`GetSelectedObject()` は現在選択されている `SceneObject` を返します。

## SceneObject

関連ファイル:

- `Headers/Scene/SceneObject.h`

`SceneObject` は描画可能な 1 つのオブジェクトを表す構造体です。

```cpp
struct SceneObject
{
    ClassicLit::MeshGroup meshGroup;
    Transform transform;
};
```

このプロジェクトでは、1 つのシーンオブジェクトは以下の組み合わせとして扱われます。

```text
SceneObject
  -> MeshGroup
     GPU resource、shader、material、render setting

  -> Transform
     translation、rotation、scaling
```

`SceneObject` 自体は薄い構造体であり、実際の描画リソースは `MeshGroup`、位置・回転・スケールは `Transform` が担当します。

## Transform

関連ファイル:

- `Headers/Scene/Transform.h`

`Transform` は object の移動、回転、拡大縮小を保持します。

```cpp
Vector3 translation;
Vector3 rotation;
Vector3 scaling;
```

`GetModelMatrix()` により model matrix を生成します。

```cpp
Matrix GetModelMatrix() const noexcept
{
    return Matrix::CreateScale(scaling) *
           Matrix::CreateRotationY(rotation.y) *
           Matrix::CreateRotationX(rotation.x) *
           Matrix::CreateRotationZ(rotation.z) *
           Matrix::CreateTranslation(translation);
}
```

回転は Y、X、Z の順に適用されます。

## Material

関連ファイル:

- `Headers/Scene/Material.h`

`Material` は classic lighting と IBL shading で使用する material 情報です。

```cpp
struct Material
{
    Vector3 ambient;
    float shininess;
    Vector3 diffuse;
    float padding0;
    Vector3 specular;
    float padding1;
    Vector3 fresnelR0;
    float padding2;
};
```

HLSL 側の material layout と一致させるため、`Vector3` の後に padding が入っています。

`Material` は `ClassicLit::RenderSettings` の中に含まれ、各 `MeshGroup` が個別に保持します。

## Light

関連ファイル:

- `Headers/Scene/Light.h`

対応する light type は以下です。

```cpp
enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2,
    Count
};
```

`Light` は directional、point、spot light で使用する値をまとめた構造体です。

```cpp
struct Light
{
    Vector3 strength;
    float fallOffStart;
    Vector3 direction;
    float fallOffEnd;
    Vector3 position;
    float spotPower;
};
```

`Ryudar` は GUI で選択された light type と `m_editableLight` の値を `MeshGroup` に渡します。

## Camera

関連ファイル:

- `Headers/Rendering/Camera.h`
- `Sources/Rendering/Camera.cpp`

`Camera` は view matrix と projection matrix を提供するクラスです。

主な機能は以下です。

- view matrix 計算
- projection matrix 計算
- camera position 取得
- mouse 入力による pitch / yaw 更新
- keyboard 入力による前後左右移動
- aspect ratio 更新
- 初期 camera 状態への reset

`Camera` は `AppBase` が所有し、`Ryudar::Update()` が毎フレーム view / projection matrix を取得して rendering constant buffer に反映します。

## Geometry データ構造

geometry 関連のデータは、CPU 側データと GPU 側リソースに分けられています。

### Vertex

関連ファイル:

- `Headers/Geometry/Vertex.h`

```cpp
struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
};
```

`Vertex` は position、normal、texcoord を持つ基本頂点形式です。

### MeshData

関連ファイル:

- `Headers/Geometry/MeshData.h`

```cpp
struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string textureFilename;
};
```

`MeshData` は GPU buffer 生成前の CPU-side mesh data です。

### Mesh

関連ファイル:

- `Headers/Geometry/Mesh.h`

```cpp
struct Mesh
{
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT m_indexCount;

    ComPtr<ID3D11Buffer> vertexConstantBuffer;
    ComPtr<ID3D11Buffer> pixelConstantBuffer;

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureResourceView;
};
```

`Mesh` は GPU resource の集合です。

全体の変換フローは以下です。

```text
GeometryGenerator / ModelLoader
  -> MeshData
  -> ClassicLit::MeshGroup::Initialize()
  -> Mesh
  -> D3D11 DrawIndexed()
```

## GeometryGenerator

関連ファイル:

- `Headers/Geometry/GeometryGenerator.h`
- `Sources/Geometry/GeometryGenerator.cpp`

`GeometryGenerator` は procedural mesh 生成と外部 model 読み込みの入口を担当します。

生成できる基本形状は以下です。

- square
- box
- grid
- cylinder / truncated cone
- sphere
- tetrahedron
- icosahedron
- subdivided sphere

外部モデルは `ReadFromFile()` から読み込みます。

```cpp
static vector<MeshData> ReadFromFile(
    const std::string &basePath,
    const std::string &fileName);
```

`ReadFromFile()` は内部で `ModelLoader` を使用し、読み込んだモデル全体を原点基準の単位サイズに正規化します。

## ModelLoader

関連ファイル:

- `Headers/Assets/ModelLoader.h`
- `Sources/Assets/ModelLoader.cpp`

`ModelLoader` は Assimp を使用して外部モデルファイルを読み込み、プロジェクト共通の `MeshData` 形式に変換します。

処理フローは以下です。

```text
Assimp::Importer::ReadFile()
  -> root node から再帰的に走査
  -> node transform を累積
  -> aiMesh を MeshData へ変換
  -> material の diffuse texture path を取得
```

`ProcessNode()` は node 階層を走査しながら parent transform を累積します。

`ProcessMesh()` は Assimp の mesh から以下の情報を取り出します。

- vertex position
- normal
- texture coordinate
- face index
- diffuse texture filename

## ClassicLit::MeshGroup

関連ファイル:

- `Headers/Rendering/ClassicLit/ClassicLitMeshGroup.h`
- `Sources/Rendering/ClassicLit/ClassicLitMeshGroup.cpp`

`ClassicLit::MeshGroup` は通常の 3D mesh rendering を担当する中心クラスです。

複数の `Mesh` を 1 つの描画単位として管理できます。FBX のように submesh が複数あるモデルでも、`std::vector<Mesh>` として扱われます。

主な責務は以下です。

- `MeshData` から GPU `Mesh` への変換
- vertex buffer / index buffer 生成
- texture 読み込み
- classic lit vertex shader / pixel shader 生成
- input layout 生成
- sampler state 生成
- vertex / lighting / shading constant buffer 管理
- material / render setting 管理
- IBL cubemap SRV の保持
- normal vector debug rendering

主要状態は以下です。

```text
Transform constants
  VertexConstantData m_vertexConstantData

Lighting constants
  LightingConstantData m_lightingConstantData

Material / shading constants
  ShadingConstantData m_shadingConstantData
  RenderSettings m_renderSettings

Mesh resources
  std::vector<Mesh> m_meshes

Shaders
  ClassicLitVertexShader
  ClassicLitPixelShader
  NormalVertexShader
  NormalPixelShader

Environment maps
  m_diffuseIBLSRV
  m_specularIBLSRV

Normal debug
  m_normalLines
  m_drawNormals
```

`Render()` では、各 mesh に対して vertex buffer、index buffer、shader、constant buffer、texture SRV、IBL SRV を bind し、`DrawIndexed()` を呼び出します。

normal debug 表示が有効な場合は、通常 mesh 描画後に normal line mesh を line list として追加描画します。

## ClassicLit RenderSettings

関連ファイル:

- `Headers/Rendering/ClassicLit/ClassicLitRenderSettings.h`

`RenderSettings` は CPU 側の rendering 設定です。

```cpp
struct RenderSettings
{
    Material material;
    ShadingSettings shading;
    EnvironmentSettings environment;
    RimLightSettings rimLight;
};
```

GUI で編集された material、shading、IBL、rim light の設定は `RenderSettings` に保存されます。

`MeshGroup::ApplyRenderSettings()` はこの CPU-side 設定を GPU constant buffer 用の `ShadingConstantData` に変換します。

## ClassicLit Constant Data

関連ファイル:

- `Headers/Rendering/ClassicLit/ClassicLitConstantData.h`

このファイルには、C++ 側と HLSL 側で layout を一致させる必要がある constant buffer 用構造体が定義されています。

主な構造体は以下です。

- `VertexConstantData`
- `LightingConstantData`
- `RimLightConstantData`
- `ShadingOptionsConstantData`
- `EnvironmentConstantData`
- `ShadingConstantData`
- `NormalVertexConstantData`

Direct3D constant buffer は 16-byte alignment が重要なため、複数の `static_assert` で構造体サイズを確認しています。

例:

```cpp
static_assert(sizeof(LightingConstantData) == 160,
              "LightingConstantData must match the HLSL cbuffer layout");
```

## CubeMapping

関連ファイル:

- `Headers/Rendering/CubeMapping.h`
- `Sources/Rendering/CubeMapping.cpp`

`CubeMapping` は skybox と IBL environment map を担当します。

主な責務は以下です。

- diffuse IBL cubemap 読み込み
- specular IBL cubemap 読み込み
- skybox 用 sphere mesh 生成
- cubemap vertex / pixel shader 生成
- camera view / projection に基づく constant buffer 更新
- skybox rendering
- 他 mesh へ IBL SRV を提供

`Ryudar::Initialize()` ではまず `CubeMapping` を初期化し、その後 sphere と character の `MeshGroup` に environment map を渡します。

```cpp
m_sphere.meshGroup.SetEnvironmentMaps(
    m_cubeMapping.m_diffuseIBLSRV.Get(),
    m_cubeMapping.m_specularIBLSRV.Get());
```

skybox 描画では specular cubemap を背景として使用します。lit mesh 側では diffuse/specular cubemap が IBL 入力として使われます。

## ImageFilter

関連ファイル:

- `Headers/Rendering/ImageFilter.h`
- `Sources/Rendering/ImageFilter.cpp`

`ImageFilter` は post-process の 1 pass を表すクラスです。

対応する filter type は以下です。

```cpp
enum class FilterType
{
    Sampling,
    BlurHorizontal,
    BlurVertical,
    Combine,
};
```

`ImageFilter` 1 つは以下を保持します。

- full-screen quad mesh
- vertex shader
- pixel shader
- input layout
- sampler state
- rasterizer state
- input shader resource views
- output render target views
- output texture shader resource view
- pass 専用 viewport
- sampling constant buffer

各 `ImageFilter` は入力 SRV を sample し、指定された RTV に結果を書き込みます。出力 texture の SRV は次の filter の入力として使用できます。

## Bloom Post Processing

Bloom の filter chain は `Ryudar::BuildFilters()` で構築されます。

処理フローは以下です。

```text
Back Buffer
  -> ResolveSubresource
  -> PostProcessInputSRV
  -> Downsample / Bright Pass
  -> Blur X
  -> Blur Y
  -> Upsample
  -> Combine original + bloom
  -> Back Buffer
```

主な流れは以下です。

1. MSAA back buffer を single-sample texture へ resolve する。
2. threshold を使って明るい領域を抽出しながら downsample する。
3. 低解像度 texture に horizontal blur と vertical blur を適用する。
4. blur 結果を段階的に upsample する。
5. 元画像と bloom 画像を combine し、back buffer へ出力する。

`m_downsampleCount` と `m_blurRepeatCount` により、downsample 段数と blur 回数が決まります。

## D3D11Utils

関連ファイル:

- `Headers/Graphics/D3D11Utils.h`
- `Sources/Graphics/D3D11Utils.cpp`

`D3D11Utils` は Direct3D 11 resource 生成をまとめた utility class です。

主な関数は以下です。

- `CreateDepthBuffer()`
- `CreateVertexShaderAndInputLayout()`
- `CreatePixelShader()`
- `CreateIndexBuffer()`
- `CreateVertexBuffer()`
- `CreateConstantBuffer()`
- `UpdateBuffer()`
- `CreateTexture()`
- `CreateCubemapTexture()`

`MeshGroup`、`CubeMapping`、`ImageFilter`、`AppBase` はこの utility を使って buffer、shader、texture などを生成します。

## D3D11Exception

関連ファイル:

- `Headers/Graphics/D3D11Exception.h`
- `Sources/Graphics/D3D11Exception.cpp`

`D3D11Exception` は Direct3D API 呼び出し失敗を例外として扱うためのクラスです。

`ThrowIfFailed()` は `HRESULT` を確認し、失敗時に `D3D11Exception` を投げます。

```cpp
void ThrowIfFailed(
    HRESULT result,
    std::string_view operation,
    std::string_view detail = {});
```

## Shader 構成

HLSL ファイルは `Shaders` フォルダにあります。

主な shader は以下です。

- `ClassicLitVertexShader.hlsl`
- `ClassicLitPixelShader.hlsl`
- `CubeMappingVertexShader.hlsl`
- `CubeMappingPixelShader.hlsl`
- `SamplingVertexShader.hlsl`
- `SamplingPixelShader.hlsl`
- `BlurXPixelShader.hlsl`
- `BlurYPixelShader.hlsl`
- `CombinePixelShader.hlsl`
- `NormalVertexShader.hlsl`
- `NormalPixelShader.hlsl`
- `Common.hlsli`

用途は以下のように分かれています。

```text
ClassicLit*
  通常 mesh rendering

CubeMapping*
  skybox / cubemap rendering

Sampling / Blur / Combine
  post-process / bloom

Normal*
  normal vector debug rendering
```

## 初期化フロー

全体の初期化は `Ryudar::Initialize()` から始まります。

大まかな流れは以下です。

```text
Ryudar::Initialize()
  -> AppBase::Initialize()
       -> InitMainWindow()
       -> InitDirect3D()
       -> InitGUI()

  -> CubeMapping::Initialize()
  -> sphere mesh 作成
  -> sphere MeshGroup 初期化
  -> character model 読み込み
  -> character MeshGroup 初期化
  -> BuildFilters()
```

`AppBase::Initialize()` が window、D3D11、ImGui を準備し、その後 `Ryudar::Initialize()` が scene-specific resource を準備します。

## 1 フレームの実行フロー

1 フレームの流れは図 3 に対応します。

### 1. AppBase::Run()

`AppBase::Run()` はメインループです。

```text
while (WM_QUIT != msg.message)
{
    PeekMessage / DispatchMessage
    or
    ImGui frame start
    UpdateGUI()
    Update(dt)
    Render()
    ImGui draw
    Present()
}
```

Win32 message がある場合は message を処理し、message がない場合は 1 frame の update / render を実行します。

### 2. Ryudar::UpdateGUI()

ImGui UI を構築します。

主な UI は以下です。

- first-person view toggle
- sphere / character selection
- shading model selection
- texture usage toggle
- wireframe toggle
- normal debug display
- bloom threshold / strength
- model transform control
- material diffuse / specular / shininess
- IBL toggle
- directional / point / spot light selection
- rim light setting

GUI は選択中 object の `MeshGroup` と `Transform` を編集します。

### 3. Ryudar::Update(float dt)

CPU 側の状態を GPU constant buffer に反映します。

主な流れは以下です。

```text
selected object を取得
  -> first-person camera 更新
  -> view / projection matrix 計算
  -> model matrix 計算
  -> inverse transpose matrix 計算
  -> light 設定
  -> eye position 設定
  -> MeshGroup constant buffer update
  -> CubeMapping constant buffer update
  -> bloom constant update if dirty
```

normal 変換には non-uniform scale に対応するため inverse transpose matrix が使用されます。

### 4. Ryudar::Render()

実際の draw call が発行されます。

主な流れは以下です。

```text
SetViewport()
ClearRenderTargetView()
ClearDepthStencilView()
OMSetRenderTargets()
Set rasterizer state

CubeMapping::Render()
GetSelectedObject().meshGroup.Render()

ResolveSubresource()

if post processing enabled:
    for each ImageFilter:
        ImageFilter::Render()

ImGui draw
Present
```

描画順は、skybox、選択中 mesh、post-process、ImGui、present です。

## 主要クラス間の関係

主要な所有関係と利用関係は以下です。

```text
main.cpp
  -> Ryudar

Ryudar
  -> AppBase を継承
  -> SceneObject m_sphere
  -> SceneObject m_character
  -> CubeMapping
  -> vector<unique_ptr<ImageFilter>>

SceneObject
  -> ClassicLit::MeshGroup
  -> Transform

ClassicLit::MeshGroup
  -> vector<Mesh>
  -> RenderSettings
  -> Constant Buffer Data
  -> D3D11 Shader / Buffer / Texture

CubeMapping
  -> Cubemap SRV
  -> Skybox Mesh

ImageFilter
  -> Full-screen Quad Mesh
  -> Input SRV
  -> Output RTV / SRV

GeometryGenerator
  -> MeshData
  -> ModelLoader

ModelLoader
  -> Assimp
  -> MeshData

D3D11Utils
  -> D3D11 resource creation helper
```

## コード確認時の推奨順序

プロジェクト全体を読む場合は、以下の順序で確認すると流れを追いやすくなります。

1. `Sources/Application/main.cpp`  
   実行開始地点を確認する。

2. `Headers/Application/AppBase.h` / `Sources/Application/AppBase.cpp`  
   window、D3D11、ImGui、main loop の基盤を確認する。

3. `Headers/Application/Ryudar.h` / `Sources/Application/Ryudar.cpp`  
   scene 初期化、GUI、update、render の流れを確認する。

4. `Headers/Scene/*.h`  
   `SceneObject`、`Transform`、`Material`、`Light` のデータ構造を確認する。

5. `Headers/Rendering/ClassicLit/*` / `Sources/Rendering/ClassicLit/ClassicLitMeshGroup.cpp`  
   通常 mesh rendering、material、lighting、constant buffer を確認する。

6. `Headers/Rendering/CubeMapping.h` / `Sources/Rendering/CubeMapping.cpp`  
   skybox と IBL resource の流れを確認する。

7. `Headers/Rendering/ImageFilter.h` / `Sources/Rendering/ImageFilter.cpp`  
   post-process pass と bloom filter chain を確認する。

8. `Headers/Geometry/*` / `Sources/Geometry/GeometryGenerator.cpp`  
   procedural geometry と mesh data の流れを確認する。

9. `Headers/Assets/ModelLoader.h` / `Sources/Assets/ModelLoader.cpp`  
   FBX model loading と Assimp の処理を確認する。

10. `Shaders/*.hlsl`  
    C++ 側 constant buffer と HLSL 側 shader 処理の対応を確認する。

## まとめ

Ryudar は、`AppBase` が Win32 / Direct3D 11 / ImGui の実行基盤を提供し、`Ryudar` が具体的な scene と rendering order を管理する構造です。

シーン内の object は `SceneObject` として表現され、`MeshGroup` が GPU resource、shader、material、light、IBL などの通常 mesh rendering を担当します。

環境マップは `CubeMapping`、後処理は `ImageFilter` と `BuildFilters()` によって構成されます。

全体としては、DirectX 11 の基本的な rendering pipeline、外部 model loading、material / light control、cubemap、post-processing を一つのデモアプリケーションとしてまとめた構成です。

