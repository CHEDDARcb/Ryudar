# Ryudar 2.0

Ryudar 2.0 は、既存の Ryudar 1.0 をベースに、よりゲームエンジンらしい構造へリファクタリングするための開発ブランチです。

Ryudar 1.0 では、DirectX 11 を用いたリアルタイム 3D レンダリング、Lighting、Material、Cubemap、IBL、Bloom、ImGui による操作 UI などを 1 つのレンダリングアプリケーションとして実装しました。

Ryudar 2.0 では、その実装を維持しながら、アプリケーション固有の処理とエンジン基盤を分離し、今後の拡張に耐えられる構造へ整理していきます。

## 機能別デモ GIF

各 GIF は `Docs/gifs/` に配置すると、README から参照しやすくなります。
実際の GIF を追加した後、以下の埋め込み例を画像表示用の Markdown として使用します。

### Shading / Light

> 推奨 GIF: `Docs/gifs/feature_shading_light.gif`
>
> `![Shading and light demo](./Docs/gifs/feature_shading_light.gif)`

Phong / Blinn-Phong、Directional / Point / Spot Light の切り替えが分かる短い GIF を配置します。

### IBL / Skybox

> 推奨 GIF: `Docs/gifs/feature_ibl_skybox.gif`
>
> `![IBL and skybox demo](./Docs/gifs/feature_ibl_skybox.gif)`

Environment Reflection、Diffuse IBL、Specular IBL、Skybox の見た目の変化を見せます。

### Model / Material

> 推奨 GIF: `Docs/gifs/feature_model_material.gif`
>
> `![Model and material demo](./Docs/gifs/feature_model_material.gif)`

Sphere と Character の切り替え、Texture、Material、Transform 操作を見せます。

### Object Selection

![Object selection demo](https://github.com/user-attachments/assets/d5e9f9c8-4d56-4997-b5c7-7304fe75fc56)

GUI で選択した object だけに Transform 操作が反映される様子を見せます。

### Normal / Wireframe

> 推奨 GIF: `Docs/gifs/feature_debug_view.gif`
>
> `![Normal and wireframe demo](./Docs/gifs/feature_debug_view.gif)`

Normal 可視化、Wireframe 表示、デバッグ用描画の切り替えを見せます。

### Bloom / Post Process

> 推奨 GIF: `Docs/gifs/feature_bloom_postprocess.gif`
>
> `![Bloom post process demo](./Docs/gifs/feature_bloom_postprocess.gif)`

Threshold、Strength、Blur の調整で Bloom の変化が分かる GIF を配置します。

### Camera / Window Resize

> 推奨 GIF: `Docs/gifs/feature_camera_resize.gif`
>
> `![Camera and resize demo](./Docs/gifs/feature_camera_resize.gif)`

First-Person Camera の移動と、ウィンドウリサイズ時の描画更新を見せます。

## 目的

Ryudar 2.0 の目的は、Ryudar 1.0 の機能を単なるデモアプリケーションとしてではなく、再利用可能なゲームエンジン風アーキテクチャへ移行することです。

主な方針は以下です。

- Application と Engine の責務を分離する
- Scene、Object、Component、Renderer の関係を整理する
- Rendering pipeline をより明確な単位へ分割する
- Material、Mesh、Texture、Shader などの resource 管理を整理する
- Post Process を独立した pipeline として扱いやすくする
- GUI 操作と engine runtime state の境界を明確にする
- 将来的な scene 拡張、複数 object 管理、renderer 差し替えに備える

## ベースライン

Ryudar 1.0 の基準点は Git tag として保存しています。

```text
ryudar-v1.0
```

Ryudar 1.0 の状態を確認したい場合は、以下で参照できます。

```bash
git switch --detach ryudar-v1.0
```

再び Ryudar 2.0 の開発ブランチへ戻る場合は以下を使用します。

```bash
git switch ryudar-2.0-engine-refactor
```

## 開発ブランチ

Ryudar 2.0 のリファクタリング作業は、以下のブランチで進めます。

```text
ryudar-2.0-engine-refactor
```

このブランチは `main` から分岐しています。

## リファクタリング予定

現時点で想定している主な変更は以下です。

- `AppBase` の役割を engine runtime と application layer に分ける
- `Ryudar` に集中している scene 初期化、GUI、render flow を分割する
- `SceneObject` をより拡張しやすい scene entity として整理する
- `ClassicLit::MeshGroup` の責務を renderer、mesh resource、material state に分ける
- `CubeMapping` を environment / skybox resource として扱いやすくする
- `ImageFilter` と Bloom 構築処理を post-process pipeline として整理する
- `D3D11Utils` の低レベル API helper と engine-level resource creation を分ける
- ドキュメントを Ryudar 2.0 の構造に合わせて更新する

## 現在の状態

この README は Ryudar 2.0 リファクタリング開始時点の案内です。

今後、実装の分割が進むにつれて、以下の内容を更新していきます。

- 新しいフォルダー構成
- Engine / Application の境界
- Scene 管理方式
- Rendering pipeline
- Resource 管理方式
- Build / Run 手順

